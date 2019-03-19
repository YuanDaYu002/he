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
#include "hisfc350.h"

/*---------------------------------------------------------------------------*/
/* hisfc350_read_reg */
/*---------------------------------------------------------------------------*/
uint8_t hisfc350_read_reg(struct spi *spi, uint8_t cmd)
{
    unsigned char status;
    unsigned int reg;
    struct spinor_host *host = (struct spinor_host *)spi->host;

    reg = cmd;
    reg_write(host, reg, HISFC350_CMD_INS);

    reg = HISFC350_CMD_CONFIG_DATA_CNT(SPI_NOR_SR_LEN)
        | HISFC350_CMD_CONFIG_RW_READ
        | HISFC350_CMD_CONFIG_DATA_EN
        | HISFC350_CMD_CONFIG_SEL_CS(spi->cs)
        | HISFC350_CMD_CONFIG_START;
    reg_write(host, reg, HISFC350_CMD_CONFIG);

    HISFC350_CMD_WAIT_CPU_FINISH(host);

    status = reg_read(host, HISFC350_CMD_DATABUF0);

    return status;
}

/*
    Send write enable cmd to SPI Nor, status[C0H]:[2]bit WEL must be set 1
*/
int spinor_general_write_enable(struct spi *spi)
{
    struct spinor_host *host = (struct spinor_host *)spi->host;
    int reg;

    reg_write(host, SPI_CMD_WREN, HISFC350_CMD_INS);

    reg = HISFC350_CMD_CONFIG_SEL_CS(spi->cs)
            | HISFC350_CMD_CONFIG_START;
    reg_write(host, reg, HISFC350_CMD_CONFIG);

    HISFC350_CMD_WAIT_CPU_FINISH(host);

    return 0;
}

/*
  some chip don't QUAD enable
*/
int spinor_qe_not_enable(struct spi *spi)
{
    return 0;
}

/*
    enable QE bit if QUAD read write is supported by SPI
*/
int spinor_general_qe_enable(struct spi *spi)
{
    unsigned char status, config, op;
    unsigned int reg;
    struct spinor_host *host = (struct spinor_host *)spi->host;

    op = hisfc350_spi_is_quad(spi);

    config = hisfc350_read_reg(spi, SPI_CMD_RDCR);
    if (((config & SPI_NOR_CR_QE_MASK) >> SPI_NOR_CR_QE_SHIFT) == op) {
        return op;
    }

    status = hisfc350_read_reg(spi, SPI_CMD_RDSR);
    reg = (config << SPI_NOR_CR_SHIFT) | status;

    spi->driver->write_enable(spi);

    if (op)
        reg |= (SPI_NOR_CR_QE_MASK << SPI_NOR_CR_SHIFT);
    else
        reg &= ~(SPI_NOR_CR_QE_MASK << SPI_NOR_CR_SHIFT);
    reg_write(host, reg, HISFC350_CMD_DATABUF0);

    reg_write(host, SPI_CMD_WRSR, HISFC350_CMD_INS);

    reg = HISFC350_CMD_CONFIG_DATA_CNT(SPI_NOR_SR_LEN + SPI_NOR_CR_LEN)
        | HISFC350_CMD_CONFIG_DATA_EN
        | HISFC350_CMD_CONFIG_SEL_CS(spi->cs)
        | HISFC350_CMD_CONFIG_START;
    reg_write(host, reg, HISFC350_CMD_CONFIG);

    HISFC350_CMD_WAIT_CPU_FINISH(host);

    return op;
}

/*
 *   enable 4byte adress for SPI which memory more than 16M
 *   */
int spinor_general_entry_4addr(struct spi *spi, int enable)
{
    struct spinor_host *host = (struct spinor_host *)spi->host;
    int reg;

    if (spi->addrcycle != SPI_4BYTE_ADDR_LEN) {
        MTD_PR(AC_DBG, "\t* Flash isn't support entry 4-byte mode.\n");
        return 0;
    }
    if (enable) {
        reg_write(host, SPI_CMD_EN4B, HISFC350_CMD_INS);
    }
    else {
        reg_write(host, SPI_CMD_EX4B, HISFC350_CMD_INS);
    }

    reg = HISFC350_CMD_CONFIG_SEL_CS(spi->cs)
            | HISFC350_CMD_CONFIG_START;
    reg_write(host, reg, HISFC350_CMD_CONFIG);

    HISFC350_CMD_WAIT_CPU_FINISH(host);

    host->set_host_addr_mode(host, enable);

    return 0;
}

/*
    Read status[C0H]:[0]bit OIP, judge whether the device is busy or not
*/
int spinor_general_wait_ready(struct spi *spi)
{
    unsigned char status;

    do {
        status = hisfc350_read_reg(spi, SPI_CMD_RDSR);
        if (!(status & SPI_CMD_SR_WIP))
            return 0;
    } while (1);

    return 1;
}

/*
 *   configure prepared for dma or bus read or write mode
 *   */
int spinor_general_bus_prepare(struct spi *spi, int op)
{
    unsigned int reg = 0;
    struct spinor_host *host = (struct spinor_host *)spi->host;

    reg |= HISFC350_BUS_CONFIG1_WRITE_INS(spi->write->cmd);
    reg |= HISFC350_BUS_CONFIG1_WRITE_DUMMY_CNT(spi->write->dummy);
    reg |= HISFC350_BUS_CONFIG1_WRITE_IF_TYPE(spi->write->iftype);

    reg |= HISFC350_BUS_CONFIG1_READ_PREF_CNT(0);
    reg |= HISFC350_BUS_CONFIG1_READ_INS(spi->read->cmd);
    reg |= HISFC350_BUS_CONFIG1_READ_DUMMY_CNT(spi->read->dummy);
    reg |= HISFC350_BUS_CONFIG1_READ_IF_TYPE(spi->read->iftype);
    reg_write(host, reg, HISFC350_BUS_CONFIG1);

    reg = HISFC350_BUS_CONFIG2_WIP_LOCATE(0);
    reg_write(host, reg, HISFC350_BUS_CONFIG2);
    if (op == READ)
        host->set_system_clock(spi->read->clock, TRUE);
    else if (op == WRITE)
        host->set_system_clock(spi->write->clock, TRUE);

    return 0;
}

