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

#include "host_common.h"
#include "spi_common.h"

#include "hifmc_common.h"
#include "hifmc100.h"
/*****************************************************************************/
/* SpanSion SPI Nor Flash "S25FL256S" Bank Address Register command */
#define SS_SPI_CMD_BRRD            0x16    /* Read Bank Register */
#define SS_SPI_CMD_BRWR            0x17    /* Write Bank Register */

/* Bank Address Register length(byte) */
#define SS_SPI_NOR_BR_LEN        1

/* Extended Address Enable bit[7] include in Bank Address Register */
#define SS_SPI_NOR_BR_EAE_SHIFT        7
#define SS_SPI_NOR_BR_EAE_MASK        (1 << SS_SPI_NOR_BR_EAE_SHIFT)
#define SS_SPI_NOR_GET_EAE_BY_BR(br)    (((br) & SS_SPI_NOR_BR_EAE_MASK) \
                        >> SS_SPI_NOR_BR_EAE_SHIFT)

extern u_char hifmc100_read_reg(struct spi *spi, u_char cmd);
/*
  enable 4byte address mode for SpanSion "s25fl256" SPI Nor
*/
int spi_s25fl256s_entry_4addr(struct spi *spi, int enable)
{
    unsigned char bank;
    unsigned int reg;
    const char *str[] = {"Disable", "Enable"};
    struct spinor_host *host = (struct spinor_host *)spi->host;

    MTD_PR(AC_DBG, "\t* Start SpanSion SPI Nor %s 4-byte mode.\n",
            str[enable]);

    if (spi->addrcycle != SPI_4BYTE_ADDR_LEN) {
        MTD_PR(AC_DBG, "\t* Flash isn't support 4-byte mode.\n");
        return 0;
    }

    /* Read old Bank Register value */
    bank = hifmc100_read_reg(spi, SS_SPI_CMD_BRRD);
    MTD_PR(AC_DBG, "\t  Read Bank Register[%#x]%#x\n", SS_SPI_CMD_BRRD,
            bank);
    if (SS_SPI_NOR_GET_EAE_BY_BR(bank) == enable) {
        MTD_PR(AC_DBG, "\t* 4-byte was %sd, bank:%#x\n", str[enable],
                bank);
        return 0;
    }

    /* Write new Bank Register value */
    if (enable)
        bank |= SS_SPI_NOR_BR_EAE_MASK;
    else
        bank &= ~SS_SPI_NOR_BR_EAE_MASK;
    writeb(bank, host->membase);

    reg = FMC_CMD_CMD1(SS_SPI_CMD_BRWR);
    reg_write(host, reg, FMC_CMD);

    reg = OP_CFG_FM_CS(spi->cs);
    reg_write(host, reg, FMC_OP_CFG);

    reg = FMC_DATA_NUM_CNT(SS_SPI_NOR_BR_LEN);
    reg_write(host, reg, FMC_DATA_NUM);

    reg = FMC_OP_CMD1_EN(ENABLE)
        | FMC_OP_WRITE_DATA_EN(ENABLE)
        | FMC_OP_REG_OP_START;
    reg_write(host, reg, FMC_OP);

    FMC_CMD_WAIT_CPU_FINISH(host);

    host->set_host_addr_mode(host, enable);

    spi->driver->wait_ready(spi);

    /* Check out Bank Register value */
    bank = hifmc100_read_reg(spi, SS_SPI_CMD_BRRD);
    MTD_PR(AC_DBG, "\t  Read Bank Register[%#x]%#x\n", SS_SPI_CMD_BRRD,
            bank);
    if (SS_SPI_NOR_GET_EAE_BY_BR(bank) == enable)
        MTD_PR(AC_DBG, "\t  %s 4byte success, bank:%#x.\n",
                str[enable], bank);
    else
        ERR_MSG(" %s 4bytes failed! bank: %#x\n", str[enable],
                bank);

    MTD_PR(AC_DBG, "\t* End SpanSion SPI Nor %s 4-byte mode.\n",
            str[enable]);

    return 0;
}

