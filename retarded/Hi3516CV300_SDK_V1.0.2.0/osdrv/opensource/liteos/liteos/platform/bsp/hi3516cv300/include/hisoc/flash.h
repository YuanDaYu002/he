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

#ifndef __HISOC_FLASH_H__
#define __HISOC_FLASH_H__

#include "asm/platform.h"
#include "hisoc/sys_ctrl.h"
#include "asm/io.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define PERI_CRG48                      0xc0
#define PERI_CLK_SEL_MASK               (0x3 << 2)
#define PERI_CRG48_CLK_SEL(_clk)        (((_clk) & 0x3) << 2)
#define PERI_CLK_SEL_24M                PERI_CRG48_CLK_SEL(0x0)
#define PERI_CLK_SEL_148_5M             PERI_CRG48_CLK_SEL(0x1)
#define PERI_CLK_SEL_125M               PERI_CRG48_CLK_SEL(0x2)
#define PERI_CLK_SEL_198M               PERI_CRG48_CLK_SEL(0x3)
#define PERI_CRG48_CLK_EN               (1 << 1)
#define PERI_CRG48_SOFT_RST_REQ         (1 << 0)

#define GET_FMC_BOOT_MODE ({ \
        unsigned int boot_mode = 0, base = (SYS_CTRL_REG_BASE); \
        boot_mode = readl((void *)(base + SYS_CTRL_SYSSTAT)); \
        boot_mode &= SPI_NOR_ADDRESS_MODE_MASK; \
        boot_mode; })

#define MULTIMODE_SHIFT                 7
/* bit[3]=0; bit[7]:SPI nor address mode; bit[7]=(0:3-Byte | 1:4-Byte) */
#define SPI_NOR_ADDRESS_MODE_MASK       (0x1 << MULTIMODE_SHIFT)
/* bit[3]=1; bit[7]: SPI nand I/O widthe; bit[7]=(0: 1-I/O | 1: 4-I/O */
#define SPI_NAND_IO_WIDTHE_MASK         (0x1 << MULTIMODE_SHIFT)

#define GET_CLK_TYPE(_reg)              (((_reg) >> 2) & 0x3)

/*---------------------------------------------------------------------------*/
/* hifmc100_set_system_clock */
/*---------------------------------------------------------------------------*/
static void hifmc100_set_system_clock(unsigned clock, int clk_en)
{
    unsigned int base = (CRG_REG_BASE);
    unsigned int old_val, regval = readl((void *)(base + PERI_CRG48));
    old_val = regval;

    regval &= ~PERI_CLK_SEL_MASK;
    regval |= PERI_CLK_SEL_24M;        /* Default Clock */

    regval &= ~PERI_CLK_SEL_MASK;
    regval |= clock & PERI_CLK_SEL_MASK;

    if (clk_en)
        regval |= PERI_CRG48_CLK_EN;
    else
        regval &= ~PERI_CRG48_CLK_EN;

    if (regval != old_val)
        writel(regval, (void *)(base + PERI_CRG48));
}

/*---------------------------------------------------------------------------*/
/* hifmc100_get_best_clock */
/*---------------------------------------------------------------------------*/
static void hifmc100_get_best_clock(unsigned int *clock)
{
    int ix;
    int clk_reg;

#define CLK_2X(_clk)    (((_clk) + 1) >> 1)
    unsigned int sysclk[] = {
        CLK_2X(24), PERI_CLK_SEL_24M,
        CLK_2X(125), PERI_CLK_SEL_125M,
        CLK_2X(148), PERI_CLK_SEL_148_5M,
        //CLK_2X(198), PERI_CLK_SEL_198M, /* timing not support gear */
        0,        0,
        0, 0,
    };
#undef CLK_2X

    clk_reg = PERI_CLK_SEL_24M;
    for (ix = 0; sysclk[ix]; ix += 2) {
        if (*clock < sysclk[ix])
            break;
        clk_reg = sysclk[ix + 1];
    }

    *clock = clk_reg;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

