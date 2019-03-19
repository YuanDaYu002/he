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
#ifndef __HISOC_MMC_H_
#define __HISOC_MMC_H_

/************************************************************************/

#include "asm/platform.h"
#include "mmc/mmc_os_adapt.h"
#include "mmc/host.h"
#include "himci_reg.h"
#include "himci.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/************************************************************************/
#define EMMC_DDR50
#define EMMC_HS200

#define MMC0    0
#define MMC1    1
#define MMC2    2
#define MMC3    3

#define MAX_MMC_NUM    4

#define MMC_FREQ_99M       99000000
#define MMC_FREQ_49_5M     49500000

#define CONFIG_MMC0_CCLK_MIN    60000      //60KHz
#define CONFIG_MMC0_CCLK_MAX    MMC_FREQ_49_5M
#define CONFIG_MMC1_CCLK_MIN    60000      //60KHz
#define CONFIG_MMC1_CCLK_MAX    MMC_FREQ_49_5M
#define CONFIG_MMC2_CCLK_MIN    60000      //60KHz
#define CONFIG_MMC2_CCLK_MAX    MMC_FREQ_99M 
#define CONFIG_MMC3_CCLK_MIN    60000      //60KHz
#define CONFIG_MMC3_CCLK_MAX    MMC_FREQ_49_5M

#define CONFIG_MMC0_CLK        MMC_FREQ_49_5M
#define CONFIG_MMC1_CLK        MMC_FREQ_49_5M
#define CONFIG_MMC2_CLK        MMC_FREQ_99M
#define CONFIG_MMC3_CLK        MMC_FREQ_49_5M

#define CONFIG_MAX_BLK_COUNT    2048
#define CONFIG_MAX_BLK_SIZE     512

#define HIMMC_PAGE_SIZE    4096

/* register mapping*/
#define PERI_CRG49              (CRG_REG_BASE + 0xC4)
#define PERI_CRG50              (CRG_REG_BASE + 0xC8)


/* sdio2:eMMC pad ctrl reg */
#define REG_CTRL_EMMC_CCLK    (IO_CTL_REG_BASE + 0xa0)
#define REG_CTRL_EMMC_CCMD    (IO_CTL_REG_BASE + 0x9c)
#define REG_CTRL_EMMC_CDATA0  (IO_CTL_REG_BASE + 0x90)
#define REG_CTRL_EMMC_CDATA1  (IO_CTL_REG_BASE + 0x98)
#define REG_CTRL_EMMC_CDATA2  (IO_CTL_REG_BASE + 0xa4)
#define REG_CTRL_EMMC_CDATA3  (IO_CTL_REG_BASE + 0x8c)

#define EMMC_CLK_DS_1V8         0xc0
#define EMMC_CMD_DS_1V8         0x150
#define EMMC_DATA0_DS_1V8       0x1d0
#define EMMC_DATA1_DS_1V8       0x1d0
#define EMMC_DATA2_DS_1V8       0x1d0
#define EMMC_DATA3_DS_1V8       0x1d0

/* clock cfg*/
#define SDIO0_CLK_SEL_MASK      (3U << 4)
#define SDIO0_CLK_SEL_49_5M     (0U << 4) /* 49.5MHz */
#define SDIO0_CKEN              (1U << 1)
#define SDIO0_RESET             (1U << 0)

#define SDIO1_CLK_SEL_MASK      (3U << 12)
#define SDIO1_CLK_SEL_49_5M     (0U << 12) /* 49.5MHz */
#define SDIO1_CKEN              (1U << 9)
#define SDIO1_RESET             (1U << 8)

#define SDIO2_CLK_SEL_MASK      (3U << 20)
#define SDIO2_CLK_SEL_99M       (0U << 20) /* 99MHz */
#define SDIO2_CLK_SEL_49_5M     (2U << 20) /* 49.5MHz */
#define SDIO2_CKEN              (1U << 17)
#define SDIO2_RESET             (1U << 16)

#define SDIO3_CLK_SEL_MASK      (3U << 4)
#define SDIO3_CLK_SEL_49_5M     (0U << 4) /* 49.5MHz */
#define SDIO3_CKEN              (1U << 1)
#define SDIO3_RESET             (1U << 0)


#define SDIO1_CLK_OFFSET    (2)
#define SDIO0_CLK_OFFSET    (10)

#define PHASE_SHIFT        0x2030000
#define READ_THRESHOLD_SIZE    0x2000005
#define DRV_PHASE_SHIFT             (0x4)
#define SMPL_PHASE_SHIFT            (0x1)

/*FIXME: CFG_PHASE_IN_TIMING
 * if open, it means that we have to config
 * phase when setting timing.
 */
#define CFG_PHASE_IN_TIMING
#ifdef  CFG_PHASE_IN_TIMING
#define DDR50_DRV_PHASE_CFG      (0x2)
#define SDR104_DRV_PHASE_CFG     (0x3)
#endif

#define HIMCI_EDGE_TUNING
#define HIMCI_PHASE_SCALE 8
#define TUNING_START_PHASE  0
#define TUNING_END_PHASE    7

#define PHASE_NOT_FOUND -1

#define hi_mci_detect_polarity_cfg(mmc_num)    do{ }while(0)

#define hi_mci_soft_reset(mmc_num) do { \
    unsigned int reg_value = 0; \
    if (MMC3 == mmc_num) {\
        reg_value = himci_readl(PERI_CRG50); \
        reg_value |= SDIO3_RESET; \
        himci_writel(reg_value, PERI_CRG50); \
    } else {\
    reg_value = himci_readl(PERI_CRG49); \
    if (MMC0 == mmc_num) \
        reg_value |= SDIO0_RESET; \
    else if (MMC1 == mmc_num) \
        reg_value |= SDIO1_RESET; \
    else if (MMC2 == mmc_num) \
        reg_value |= SDIO2_RESET; \
    himci_writel(reg_value, PERI_CRG49); \
    }\
    mmc_delay_us(100); \
    if (MMC3 == mmc_num) {\
        reg_value &= (~SDIO3_RESET); \
        himci_writel(reg_value, PERI_CRG50); \
    } else {\
    if (MMC0 == mmc_num) \
        reg_value &= (~SDIO0_RESET); \
    else if (MMC1 == mmc_num) \
        reg_value &= (~SDIO1_RESET); \
    else if (MMC2 == mmc_num) \
        reg_value &= (~SDIO2_RESET); \
    himci_writel(reg_value, PERI_CRG49); \
    }\
}while(0)

/* *
 * MMC HOST useable
 * Set to 1: useable
 * Set to 0: not useable
 * */
#define USE_MMC0    1
#define USE_MMC1    1
#define USE_MMC2    1
#define USE_MMC3    0

#define USE_THIS_MMC(mmc_num) ({ \
    int value = 0; \
    if (MMC0 == mmc_num) \
        value = USE_MMC0; \
    else if (MMC1 == mmc_num) \
        value = USE_MMC1; \
    else if (MMC2 == mmc_num) \
        value = USE_MMC2; \
    else if (MMC3 == mmc_num) \
        value = USE_MMC3; \
    value; \
})
//#define CONFIG_SEND_AUTO_STOP /* open auto stop */

/* MCI_FIFOTH(0x4c) details */
#define BURST_SIZE        (0x6<<28)
#define RX_WMARK        (0x7f<<16)
#define TX_WMARK        0x80
#define MCI_BMOD_VALUE  (BURST_INCR | BURST_16)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif

