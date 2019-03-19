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

#ifndef    __ASM_PLATFORM_H__
#define    __ASM_PLATFORM_H__

#include "asm/hal_platform_ints.h"
#include "hisoc/timer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define DDR_MEM_BASE              0x80000000

#define FMC_MEM_BASE              0x14000000

#define GPIO8_REG_BASE            0x12148000
#define GPIO7_REG_BASE            0x12147000
#define GPIO6_REG_BASE            0x12146000
#define GPIO5_REG_BASE            0x12145000
#define GPIO4_REG_BASE            0x12144000
#define GPIO3_REG_BASE            0x12143000
#define GPIO2_REG_BASE            0x12142000
#define GPIO1_REG_BASE            0x12141000
#define GPIO0_REG_BASE            0x12140000

#define PWM47_REG_BASE            0x12130000
#define PWM03_REG_BASE            0x12130000

#define SPI_3WIRE_REG_BASE        0x12122000
#define SPI1_REG_BASE             0x12121000
#define SPI0_REG_BASE             0x12120000

#define I2C1_REG_BASE             0x12112000
#define I2C0_REG_BASE             0x12110000

#define UART2_REG_BASE            0x12102000
#define UART1_REG_BASE            0x12101000
#define UART0_REG_BASE            0x12100000

#define PMC_REG_BASE              0x120A0000
#define RTC_REG_BASE              0x12090000
#define WDG_REG_BASE              0x12080000

#define MDDRC_REG_BASE            0x12060000
#define IO_CTL_REG_BASE           0x12040800
#define IO_MUX_REG_BASE           0x12040000
#define MISC_REG_BASE             0x12030000
#define SYS_CTRL_REG_BASE         0x12020000
#define CRG_REG_BASE              0x12010000

#define USBDEV_REG_BASE           0x10130000
#define USB_EHCI_REG_BASE         0x10120000
#define USB_OHCI_REG_BASE         0x10110000

#define SDIO3_REG_BASE            0x100F0000
#define SDIO2_REG_BASE            0x100E0000
#define SDIO1_REG_BASE            0x100D0000
#define SDIO0_REG_BASE            0x100C0000

#define HASH_REG_BASE             0x10090000
#define CIPHER_REG_BASE           0x10080000
#define ETH_REG_BASE              0x10050000

#define VIC_REG_BASE              0x10040000
#define IRQ_REG_BASE              VIC_REG_BASE

#define DMAC_REG_BASE             0x10030000
#define FMC_REG_BASE              0x10000000

#define BIT(n)  (1U << (n))
#define TIMER0_ENABLE   BIT(16)
#define TIMER1_ENABLE   BIT(18)
#define TIMER2_ENABLE   BIT(20)
#define TIMER3_ENABLE   BIT(22)

#define TIMER3_REG_BASE           0x12001020
#define TIMER2_REG_BASE           0x12001000
#define TIMER1_REG_BASE           0x12000020
#define TIMER0_REG_BASE           0x12000000
#define TIMER_TICK_REG_BASE       TIMER0_REG_BASE   /* timer for tick */
#define TIMER_TICK_ENABLE   TIMER0_ENABLE
#define TIMER_TIME_REG_BASE       TIMER1_REG_BASE   /* timer for time */
#define TIMER_TIME_ENABLE   TIMER1_ENABLE
#define HRTIMER_TIMER_REG_BASE  TIMER3_REG_BASE /* timer for hrtimer */
#define HRTIMER_TIMER_ENABLE    TIMER3_ENABLE

#define RAM_BASE                  0x04010000
#define BOOTROM_BASE              0x04000000
#define REMAP_MEM_BASE            0x00000000

#define CACHE_ALIGNED_SIZE        32

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif

