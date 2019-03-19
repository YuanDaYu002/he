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

#define GD_SPI_CMD_RDSR1        0x35    /* Read Status Register-1 */

extern u_char hifmc100_read_reg(struct spi *spi, u_char cmd);
/*
    enable QE bit if QUAD read write is supported by GD "25qxxx" SPI
*/
int spi_gd25qxxx_qe_enable(struct spi *spi)
{
    unsigned char config, status, op;
    unsigned int reg;
    const char *str[] = {"Disable", "Enable"};
    struct spinor_host *host = (struct spinor_host *)spi->host;

    op = spi_is_quad(spi);

    MTD_PR(QE_DBG, "\t*-Start GD SPI nor %s Quad.\n", str[op]);
    config = hifmc100_read_reg(spi, GD_SPI_CMD_RDSR1);
    MTD_PR(QE_DBG, "\t|-Read GD SR-1[%#x], val: %#x\n", GD_SPI_CMD_RDSR1,
            config);
    if (SPI_NOR_GET_QE_BY_CR(config) == op) {
        MTD_PR(QE_DBG, "\t* Quad was %sd, status:%#x\n", str[op],
                config);
        return op;
    }

    /* First, we enable/disable QE for 16Pin GD flash, use WRSR[01h] cmd */
    MTD_PR(QE_DBG, "\t|-First, 16Pin GD flash %s Quad.\n", str[op]);
    status = hifmc100_read_reg(spi, SPI_CMD_RDSR);
    MTD_PR(QE_DBG, "\t|-Read Status Register[%#x]%#x\n", SPI_CMD_RDSR,
            status);

    spi->driver->write_enable(spi);

    if (op)
        config |= SPI_NOR_CR_QE_MASK;
    else
        config &= ~SPI_NOR_CR_QE_MASK;
    writeb(status, host->membase);
    writeb(config, host->membase + SPI_NOR_SR_LEN);

    reg = FMC_CMD_CMD1(SPI_CMD_WRSR);
    reg_write(host, reg, FMC_CMD);

    reg = OP_CFG_FM_CS(spi->cs);
    reg_write(host, reg, FMC_OP_CFG);

    reg = FMC_DATA_NUM_CNT(SPI_NOR_SR_LEN + SPI_NOR_CR_LEN);
    reg_write(host, reg, FMC_DATA_NUM);

    reg = FMC_OP_CMD1_EN(ENABLE)
        | FMC_OP_WRITE_DATA_EN(ENABLE)
        | FMC_OP_REG_OP_START;
    reg_write(host, reg, FMC_OP);

    FMC_CMD_WAIT_CPU_FINISH(host);

    spi->driver->wait_ready(spi);

    config = hifmc100_read_reg(spi, GD_SPI_CMD_RDSR1);
    MTD_PR(QE_DBG, "\t|-Read GD SR-1[%#x], val: %#x\n", GD_SPI_CMD_RDSR1,
            config);
    if (SPI_NOR_GET_QE_BY_CR(config) == op) {
        MTD_PR(QE_DBG, "\t|-16P %s Quad success reg: %#x\n", str[op],
                config);
        goto QE_END;
    } else
        MTD_PR(QE_DBG, "\t|-16P %s Quad failed, reg: %#x\n", str[op],
                config);

    /* Second, we enable/disable QE for 8Pin GD flash, use WRSR2[31h] cmd */
    MTD_PR(QE_DBG, "\t|-Second, 8Pin GD flash %s Quad.\n", str[op]);
    status = hifmc100_read_reg(spi, SPI_CMD_RDSR);
    MTD_PR(QE_DBG, "\t|-Read Status Register[%#x]:%#x\n", SPI_CMD_RDSR,
            status);
    if (!(status & STATUS_WEL_MASK))
        spi->driver->write_enable(spi);

    config = hifmc100_read_reg(spi, SPI_CMD_RDSR2);
    MTD_PR(QE_DBG, "\t|-Read SR-2[%#x], val: %#x\n", SPI_CMD_RDSR2,
            config);
    if (op)
        config |= SPI_NOR_CR_QE_MASK;
    else
        config &= ~SPI_NOR_CR_QE_MASK;
    writeb(config, host->membase);

    reg = FMC_CMD_CMD1(SPI_CMD_WRSR2);
    reg_write(host, reg, FMC_CMD);

    reg = OP_CFG_FM_CS(spi->cs);
    reg_write(host, reg, FMC_OP_CFG);

    reg = FMC_DATA_NUM_CNT(SPI_NOR_CR_LEN);
    reg_write(host, reg, FMC_DATA_NUM);

    reg = FMC_OP_CMD1_EN(ENABLE)
        | FMC_OP_WRITE_DATA_EN(ENABLE)
        | FMC_OP_REG_OP_START;
    reg_write(host, reg, FMC_OP);

    FMC_CMD_WAIT_CPU_FINISH(host);

    spi->driver->wait_ready(spi);

    config = hifmc100_read_reg(spi, SPI_CMD_RDSR2);
    MTD_PR(QE_DBG, "\t|-Read GD SR-2[%#x], val: %#x\n", SPI_CMD_RDSR2,
            config);
    if (SPI_NOR_GET_QE_BY_CR(config) == op)
        MTD_PR(QE_DBG, "\t|-8P %s Quad success, reg: %#x.\n", str[op],
                config);
    else
        ERR_MSG(" %s Quad failed, reg: %#x\n", str[op], config);

QE_END:

    MTD_PR(QE_DBG, "\t*-End GD SPI nor %s Quad end.\n", str[op]);

    return op;
}

