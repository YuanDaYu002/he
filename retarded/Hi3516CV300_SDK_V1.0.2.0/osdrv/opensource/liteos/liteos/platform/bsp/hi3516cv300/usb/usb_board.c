/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include "stdio.h"
#include "stdlib.h"
#include "asm/atomic.h"
#include "hisoc/usb.h"
#include "asm/delay.h"
#include "los_hwi.h"

extern VOID LOS_Udelay(UINT32 usecs);
extern VOID LOS_Mdelay(UINT32 msecs);

#define REG_CRG46                       0xb8

#define USB_CKEN                BIT(0)
#define USB_OHCI48M_CKEN        BIT(1)
#define USB_OHCI12M_CKEN        BIT(2)
#define USB_OTG_UTMI_CKEN       BIT(3)
#define USB_HST_PHY_CKEN        BIT(4)
#define USB_UTMI0_CKEN          BIT(5)
#define USB_PHY_CKEN            BIT(7)
#define USB_BUS_SRST_REQ        BIT(8)
#define USB_UTMI0_SRST_REQ      BIT(9)
#define USB_HST_PHY_SRST_REQ    BIT(11)
#define USB_OTG_PHY_SRST_REQ    BIT(12)
#define USB_PHY_REQ             BIT(13)
#define USB_PHY_PORT0_TREQ      BIT(14)
#define USB_PHY_CLKSEL          BIT(15)
#define USB_PHY_TEST_SRST_REQ   BIT(17)

#define PERI_USB                (0x12030000 + 0x5c)
#define WORDINTERFACE           BIT(0)
#define SS_BURST4_EN            BIT(7)
#define SS_BURST8_EN            BIT(8)
#define SS_BURST16_EN           BIT(9)
#define USBOVR_P_CTRL           BIT(17)
#define USB2_PHY_DPPULL_DOWN       (3 << 26)
#define DWC_OTG_EN                 (1 << 31)
#define MISC_USB                (0x12030000 + 0x64)

#define USB2_PHY_SET            0x60
#define USB_PHY_REG_ACCESS      (0x1 << 16)

static atomic_t dev_open_cnt = 0;

static int otg_usbhost_stat = 0;
static int otg_usbdev_stat = 0;

void hiusb_start_hcd(void)
{
	unsigned long flags;

	flags = LOS_IntLock();
	if (atomic_inc_return(&dev_open_cnt) == 1) {
		int reg;

		/* enable phy ref clk to enable phy */
		reg = GET_UINT32(CRG_REG_BASE + REG_CRG46);
		reg |= USB_BUS_SRST_REQ;
		reg |= USB_UTMI0_SRST_REQ;
		reg |= USB_HST_PHY_SRST_REQ;
		reg |= USB_OTG_PHY_SRST_REQ;
		reg |= USB_PHY_REQ;
		reg |= USB_PHY_PORT0_TREQ;
		reg |= USB_PHY_TEST_SRST_REQ;
		WRITE_UINT32(reg, CRG_REG_BASE + REG_CRG46);
		LOS_Udelay(100);

		/* enable phy ref clk to enable phy */
		reg = GET_UINT32(CRG_REG_BASE + REG_CRG46);
		reg |= USB_CKEN;
		reg |= USB_OHCI48M_CKEN;
		reg |= USB_OHCI12M_CKEN;
		reg |= USB_OTG_UTMI_CKEN;
		reg |= USB_HST_PHY_CKEN;
		reg |= USB_UTMI0_CKEN;
		reg |= USB_PHY_CKEN;
		WRITE_UINT32(reg, CRG_REG_BASE + REG_CRG46);
		LOS_Udelay(100);

		/* step1¡êorelease phy test reset */
		reg = GET_UINT32(CRG_REG_BASE + REG_CRG46);
		reg &= ~USB_PHY_TEST_SRST_REQ;
		WRITE_UINT32(reg, CRG_REG_BASE + REG_CRG46);
		LOS_Udelay(100);

		/* step2¡êorelease phy por reset */
		reg = GET_UINT32(CRG_REG_BASE + REG_CRG46);
		reg &= ~USB_PHY_REQ;
		WRITE_UINT32(reg, CRG_REG_BASE + REG_CRG46);
		LOS_Udelay(100);

		/* step3¡êoconfig phy clk60m */
		reg = GET_UINT32(MISC_REG_BASE + USB2_PHY_SET);
		reg |= USB_PHY_REG_ACCESS;
		WRITE_UINT32(reg, MISC_REG_BASE + USB2_PHY_SET);
		WRITE_UINT32(0x4, 0x120d0018);
		LOS_Mdelay(2);
        reg =  GET_UINT32(0x12020ee0);//get chip ID
        if((reg >> 24) == 0x4) /*hi3516ev100*/
		    WRITE_UINT32(0x1c, 0x120d0000);
        else /*hi3516cv300*/
		    WRITE_UINT32(0x18, 0x120d0000);
		LOS_Udelay(20);
		WRITE_UINT32(0xc4, 0x120d0000 + 0x20);
		LOS_Udelay(20);
		WRITE_UINT32(0xc1, 0x120d0000 + 0x44);
		LOS_Udelay(20);
		WRITE_UINT32(0x1b, 0x120d0000 + 0x28);
		LOS_Udelay(20);

		/* step4¡êorelease phy utmi reseti */
		reg = GET_UINT32(CRG_REG_BASE + REG_CRG46);
		reg &= ~USB_PHY_PORT0_TREQ;
		WRITE_UINT32(reg, CRG_REG_BASE + REG_CRG46);
		LOS_Udelay(100);

		/* step5¡êorelease controller reset */
		reg = GET_UINT32(CRG_REG_BASE + REG_CRG46);
		reg &= ~USB_BUS_SRST_REQ;
		reg &= ~USB_UTMI0_SRST_REQ;
		reg &= ~USB_HST_PHY_SRST_REQ;
		reg &= ~USB_OTG_PHY_SRST_REQ;
		WRITE_UINT32(reg, CRG_REG_BASE + REG_CRG46);
		LOS_Udelay(100);
		/* the end */
	}
    LOS_IntRestore(flags);
}

void hiusb_stop_hcd(void)
{
    unsigned long flags;

    flags = LOS_IntLock();
    if (atomic_dec_return(&dev_open_cnt) == 0) {

        int reg;

        /* Disable EHCI clock.
	   If the HS PHY is unused disable it too. */
	reg = GET_UINT32(CRG_REG_BASE + REG_CRG46);
	reg &= ~(USB_CKEN);
	reg |= USB_BUS_SRST_REQ;
	reg |= USB_UTMI0_SRST_REQ;
	reg |= USB_HST_PHY_SRST_REQ;
	reg |= USB_OTG_PHY_SRST_REQ;
	reg |= USB_PHY_REQ;
	reg |= USB_PHY_PORT0_TREQ;
	reg |= USB_PHY_TEST_SRST_REQ;
	WRITE_UINT32(reg, CRG_REG_BASE + REG_CRG46);
	LOS_Udelay(100);
    }
    LOS_IntRestore(flags);
}

void hiusb_reset_hcd(void)
{
    int reg1, reg2;
    reg1 = GET_UINT32(IO_ADDRESS(0x100b0024));
    LOS_Mdelay(1);
    reg2 = GET_UINT32(IO_ADDRESS(0x100b0028));
    LOS_Mdelay(1);
    WRITE_UINT32(0x37, IO_ADDRESS(0x200300b8));
    LOS_Msleep(100);
    WRITE_UINT32(0x80, IO_ADDRESS(0x200300b8));
    LOS_Mdelay(1);
    WRITE_UINT32(0xc06, IO_ADDRESS(0x20120080));
    LOS_Mdelay(1);
    WRITE_UINT32(0xc26, IO_ADDRESS(0x20120080));
    LOS_Mdelay(1);
    WRITE_UINT32(reg1, IO_ADDRESS(0x100b0024));
    LOS_Mdelay(1);
    WRITE_UINT32(reg2, IO_ADDRESS(0x100b0028));
    LOS_Mdelay(1);
    WRITE_UINT32(0x10005, IO_ADDRESS(0x100b0010));
    LOS_Mdelay(1);
    WRITE_UINT32(0x1, IO_ADDRESS(0x100b0050));
    LOS_Mdelay(1);
    WRITE_UINT32(0x37, IO_ADDRESS(0x100b0018));
    LOS_Mdelay(1);
    WRITE_UINT32(0x1000, IO_ADDRESS(0x100b0054));
    LOS_Mdelay(1);
}

void hiusb_host2device(void)
{
    unsigned int reg;
    unsigned int reg1;

    reg = GET_UINT32(PERI_USB);
    reg &= ~(USB2_PHY_DPPULL_DOWN);
    reg |= DWC_OTG_EN;     /*lint !e648*/
    WRITE_UINT32(reg, PERI_USB);
    reg1 = GET_UINT32(IO_ADDRESS(0x10080804));
    reg1 &= ~(0x1 << 1);
    WRITE_UINT32(reg1, IO_ADDRESS(0x10080804));
}

void hiusb_device2host(void)
{
    unsigned int reg;
    unsigned int reg1;

    reg = GET_UINT32(PERI_USB);
    reg1 = GET_UINT32(IO_ADDRESS(0x10080804));
    reg1 |= (0x1 << 1);
    WRITE_UINT32(reg1, IO_ADDRESS(0x10080804));
    reg |= USB2_PHY_DPPULL_DOWN;
    reg &= ~DWC_OTG_EN;    /*lint !e648*/
    WRITE_UINT32(reg, PERI_USB);
}

void usb_otg_run(void)
{
    int reg;
    reg = GET_UINT32(PERI_USB);

    /* device -->host */
    if ((reg & DWC_OTG_EN) == DWC_OTG_EN) {    /*lint !e648*/
        if (otg_usbdev_stat == 1) {
            return;
        }
        hiusb_device2host();

    } else { /* host -->device */
        if (otg_usbhost_stat == 1)
            return ;
        hiusb_host2device();
    }
}

int hiusb_is_device_mode(void)
{
    return ((otg_usbhost_stat == 0)&&(otg_usbdev_stat == 1));
}

void usb_otg_sw_set_host_state(void)
{
    otg_usbhost_stat = 1;
}

void usb_otg_sw_clear_host_state(void)
{
    otg_usbhost_stat = 0;
}

void usb_otg_sw_set_device_state(void)
{
    otg_usbdev_stat = 1;
}

void usb_otg_sw_clear_device_state(void)
{
    otg_usbdev_stat = 0;
}
