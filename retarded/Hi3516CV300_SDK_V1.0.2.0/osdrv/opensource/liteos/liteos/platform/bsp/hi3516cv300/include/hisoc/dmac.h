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


#ifndef __HISOC_DMAC_H__
#define __HISOC_DMAC_H__
#include "asm/io.h"
#include "asm/platform.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define DDRAM_ADRS    DDR_MEM_BASE      /* fixed */
#define DDRAM_SIZE    0x03FFFFFF        /* 64M DDR. */


#define DMAC_INTSTATUS            (DMAC_REG_BASE + 0X00)
#define DMAC_INTTCSTATUS        (DMAC_REG_BASE + 0X04)
#define DMAC_INTTCCLEAR            (DMAC_REG_BASE + 0X08)
#define DMAC_INTERRORSTATUS        (DMAC_REG_BASE + 0X0C)
#define DMAC_INTERRCLR            (DMAC_REG_BASE + 0X10)

#define DMAC_RAWINTTCSTATUS        (DMAC_REG_BASE + 0X14)
#define DMAC_RAWINTERRORSTATUS    (DMAC_REG_BASE + 0X18)

#define DMAC_ENBLDCHNS            (DMAC_REG_BASE + 0X1C)
#define DMAC_SOFT_BREQ          (DMAC_REG_BASE + 0X20)
#define DMAC_SOFT_SREQ          (DMAC_REG_BASE + 0X24)
#define DMAC_SOFT_LBREQ         (DMAC_REG_BASE + 0X28)
#define DMAC_SOFT_LSREQ         (DMAC_REG_BASE + 0X2C)

#define DMAC_CONFIG                (DMAC_REG_BASE + 0X30)
#define DMAC_SYNC                (DMAC_REG_BASE + 0X34)

/*the definition for DMAC channel register*/
#define DMAC_CxBASE(i)        (DMAC_REG_BASE + 0x100+i*0x20)

#define DMAC_CxSRCADDR(i)    (DMAC_CxBASE(i) + 0x00)
#define DMAC_CxDESTADDR(i)    (DMAC_CxBASE(i) + 0x04)
#define DMAC_CxLLI(i)        (DMAC_CxBASE(i) + 0x08)
#define DMAC_CxCONTROL(i)    (DMAC_CxBASE(i) + 0x0C)
#define DMAC_CxCONFIG(i)    (DMAC_CxBASE(i) + 0x10)

#define DMAC_MAXTRANSFERSIZE    0x0fff /*the max length is denoted by 0-11bit*/
//#define MAXTRANSFERSIZE        DMAC_MAXTRANSFERSIZE
#define DMAC_CxDISABLE            0x00
#define DMAC_CxENABLE            0x01

/*the means the bit in the channel control register*/
#define DMAC_CxCONTROL_M2M        0x8d489000  /* Dwidth=32,burst size=4 */
#define DMAC_CxCONTROL_LLIM2M    0x0d489000  /* Dwidth=32,burst size=4 */
#define DMAC_CxCONTROL_LLIM2M_ISP   0x0b489000  /* Dwidth=32,burst size=1 */
#define DMAC_CxCONTROL_LLIP2M    0x0a000000            //0x09409000
#define DMAC_CxCONTROL_LLIM2P    0x86089000

#define DMAC_CxCONTROL_M2P       0x96000000  /* Dwidth=8,burst size=1 */
#define DMAC_CxCONTROL_P2M       0x99000000  /* Dwidth=8,burst size=1 */

#define DMAC_CxCONTROL_INT_EN   (0x01 << 31)  /* bit:31,enable interrupt */

#define DMAC_CxLLI_LM            0x01
#define DMAC_TRANS_SIZE         0xff0

/*#define DMAC_CxCONFIG_M2M  0x4001*/
#define DMAC_CHANNEL_ENABLE        1
#define DMAC_CHANNEL_DISABLE    0xfffffffe

#define DMAC_CxCONFIG_M2M        0xc000
#define DMAC_CxCONFIG_LLIM2M    0xc000

#define DMAC_CxCONFIG_P2M        0xd000
#define DMAC_CxCONFIG_M2P        0xc800

#define DMAC_CxCONFIG_SIO_P2M    0x0000d000
#define DMAC_CxCONFIG_SIO_M2P    0x0000c800

/*default the config and sync regsiter for DMAC controller*/
/*M1,M2 little endian, enable DMAC*/
#define DMAC_CONFIG_VAL            0x01
/*enable the sync logic for the 16 peripheral*/
#define DMAC_SYNC_VAL            0x0

#define DMAC_MAX_PERIPHERALS    16    //12
#define MEM_MAX_NUM                1
#define CHANNEL_NUM                4
#define DMAC_MAX_CHANNELS    CHANNEL_NUM

#define PERI_CRG54  (CRG_REG_BASE + 0xd8)
#define DMAC_CLK_EN     (1 << 5)
#define DMAC_SRST_REQ   (1 << 4)


static void hidmac_clk_en(void)
{
    unsigned int tmp;

    tmp = readl(PERI_CRG54);
    tmp |= DMAC_CLK_EN;
    writel(tmp, PERI_CRG54);
}

static void hidmac_unreset(void)
{
    unsigned int tmp;

    tmp = readl(PERI_CRG54);
    tmp &= ~DMAC_SRST_REQ;
    writel(tmp, PERI_CRG54);
}

#define PERI_8BIT_MODE            0
#define PERI_16BIT_MODE           1
#define PERI_32BIT_MODE           2


//hidmac data structure

/* DMAC peripheral structure */
typedef struct dmac_peripheral {
    /* peripherial ID */
    unsigned int peri_id;
    /* peripheral data register address */
    unsigned int peri_addr;
    /* default channel control word */
    unsigned int transfer_ctrl;
    /* default channel configuration word */
    unsigned int transfer_cfg;
    /* default channel configuration word */
    unsigned int transfer_width;
} dmac_peripheral;

/*
 *    DMA config array!
 *    DREQ, FIFO, CONTROL, CONFIG, BITWIDTH
 */
static dmac_peripheral  g_peripheral[DMAC_MAX_PERIPHERALS] = {
    /* DREQ,  FIFO,   CONTROL,   CONFIG, WIDTH */
    /*periphal 0: I2C0/I2C1 RX*/
    { 0, I2C0_REG_BASE + 0x10, 0x99000000, 0xd000, PERI_8BIT_MODE},
    /*periphal 1: I2C0/I2C1 TX*/
    { 1, I2C0_REG_BASE + 0x10, 0x96000000, 0xc840, PERI_8BIT_MODE},
    /*periphal 2: reserved */
    { 0 },
    /*periphal 3: reserved */
    { 0 },
    /*periphal 4: I2C1 RX*/
    { 4, I2C1_REG_BASE + 0x00, DMAC_CxCONTROL_LLIP2M, DMAC_CxCONFIG_P2M | (4 << 1), PERI_8BIT_MODE},

    /*periphal 5: I2C1 TX*/
    { 5, I2C1_REG_BASE + 0x00, DMAC_CxCONTROL_LLIM2P, DMAC_CxCONFIG_M2P | (5 << 1), PERI_8BIT_MODE},

    /*periphal 6: UART0 RX*/
    { 6, UART0_REG_BASE + 0x00, DMAC_CxCONTROL_LLIP2M, DMAC_CxCONFIG_P2M | (6 << 1), PERI_8BIT_MODE},

    /*periphal 7: UART0 TX*/
    { 7, UART0_REG_BASE + 0x00, DMAC_CxCONTROL_LLIM2P, DMAC_CxCONFIG_M2P | (7 << 1), PERI_8BIT_MODE},

    /*periphal 8: UART1 RX*/
    { 8, UART1_REG_BASE + 0x00, DMAC_CxCONTROL_LLIP2M, DMAC_CxCONFIG_P2M | (8 << 1), PERI_8BIT_MODE},

    /*periphal 9: UART1 TX*/
    { 9, UART1_REG_BASE + 0x00, DMAC_CxCONTROL_LLIM2P, DMAC_CxCONFIG_M2P | (9 << 1), PERI_8BIT_MODE},

    /*periphal 10: UART2 RX*/
    { 10, UART2_REG_BASE + 0x00, DMAC_CxCONTROL_LLIP2M, DMAC_CxCONFIG_P2M | (10 << 1), PERI_8BIT_MODE},

    /*periphal 11: UART2 TX*/
    { 11, UART2_REG_BASE + 0x00, DMAC_CxCONTROL_LLIM2P, DMAC_CxCONFIG_M2P | (11 << 1), PERI_8BIT_MODE},

    /*periphal 12: SSP0 RX*/
    { 12, SPI0_REG_BASE + 0x08, DMAC_CxCONTROL_P2M, DMAC_CxCONFIG_P2M | (12 << 1), PERI_8BIT_MODE},

    /*periphal 13: SSP0 TX*/
    { 13, SPI0_REG_BASE + 0x08, DMAC_CxCONTROL_M2P, DMAC_CxCONFIG_M2P | (13 << 6), PERI_8BIT_MODE},

    /*periphal 14: SSP1 RX*/
    { 14, SPI1_REG_BASE + 0x08, DMAC_CxCONTROL_P2M, DMAC_CxCONFIG_P2M | (14 << 1), PERI_8BIT_MODE},

    /*periphal 15: SSP1 TX*/
    { 15, SPI1_REG_BASE + 0x08, DMAC_CxCONTROL_M2P, DMAC_CxCONFIG_M2P | (15 << 6), PERI_8BIT_MODE},

};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif
