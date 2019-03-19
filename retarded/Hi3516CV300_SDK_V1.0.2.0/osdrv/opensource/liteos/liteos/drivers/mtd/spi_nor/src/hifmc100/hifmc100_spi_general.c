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

/*---------------------------------------------------------------------------*/
/* hifmc100_read_reg */
/*---------------------------------------------------------------------------*/
uint8_t hifmc100_read_reg(struct spi *spi, uint8_t cmd)
{
    uint8_t status;
    int reg;
    struct spinor_host *host = (struct spinor_host *)spi->host;

    MTD_PR(SR_DBG, "\t * Start get flash Register[%#x]\n", cmd);
    reg = OP_CFG_FM_CS(spi->cs);
    reg_write(host, reg, FMC_OP_CFG);

    if (cmd == SPI_CMD_RDSR) {
        reg = FMC_OP_READ_STATUS_EN(ENABLE) | FMC_OP_REG_OP_START;
        goto cmd_config_done;
    }

    reg = cmd;
    reg_write(host, reg, FMC_CMD);

    reg = FMC_DATA_NUM_CNT(SPI_NOR_CR_LEN);
    reg_write(host, reg, FMC_DATA_NUM);

    reg = FMC_OP_CMD1_EN(ENABLE)
        | FMC_OP_READ_DATA_EN(ENABLE)
        | FMC_OP_REG_OP_START;
cmd_config_done:
    reg_write(host, reg, FMC_OP);

    FMC_CMD_WAIT_CPU_FINISH(host);

    if (cmd == SPI_CMD_RDSR)
        status = reg_read(host, FMC_STATUS);
    else
        status = readb(host->membase);
    MTD_PR(SR_DBG, "\t * End get flash Register[%#x], val: %#x\n", cmd,
            status);

    return status;
}

/*
    Send write enable cmd to SPI Nor, status[C0H]:[2]bit WEL must be set 1
*/
int spinor_general_write_enable(struct spi *spi)
{
    struct spinor_host *host = (struct spinor_host *)spi->host;
    int reg;
    MTD_PR(WE_DBG, "\t  * Start Write Enable\n");

    reg = hifmc100_read_reg(spi, SPI_CMD_RDSR);
    MTD_PR(WE_DBG, "\t    Read Status Register[%#x]:%#x\n", SPI_CMD_RDSR,
            reg);
    if (reg & STATUS_WEL_MASK) {
        MTD_PR(WE_DBG, "\t    Write Enable was opened! reg: %#x\n",
                reg);
        return 0;
    }

    reg = reg_read(host, FMC_GLOBAL_CFG);
    if (reg & FMC_GLOBAL_CFG_WP_ENABLE) {
        reg &= ~FMC_GLOBAL_CFG_WP_ENABLE;
        reg_write(host, reg, FMC_GLOBAL_CFG);
    }

    reg = FMC_CMD_CMD1(SPI_CMD_WREN);
    reg_write(host, reg, FMC_CMD);

    reg = OP_CFG_FM_CS(spi->cs);
    reg_write(host, reg, FMC_OP_CFG);

    reg = FMC_OP_CMD1_EN(ENABLE) | FMC_OP_REG_OP_START;
    reg_write(host, reg, FMC_OP);

    FMC_CMD_WAIT_CPU_FINISH(host);

    MTD_PR(WE_DBG, "\t  * End Write Enable\n");
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
    const char *str[] = {"Disable", "Enable"};
    struct spinor_host *host = (struct spinor_host *)spi->host;

    op = spi_is_quad(spi);

    MTD_PR(QE_DBG, "\t*-Start SPI Nor %s Quad.\n", str[op]);
    config = hifmc100_read_reg(spi, SPI_CMD_RDCR);
    MTD_PR(QE_DBG, "\t|-Read Config Register[%#x]%#x\n", SPI_CMD_RDCR,
            config);
    if (SPI_NOR_GET_QE_BY_CR(config) == op) {
        MTD_PR(QE_DBG, "\t* Quad was %sd, config:%#x\n", str[op],
                config);
        return op;
    }

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

    config = hifmc100_read_reg(spi, SPI_CMD_RDCR);
    if (SPI_NOR_GET_QE_BY_CR(config) == op)
        MTD_PR(QE_DBG, "\t|-%s Quad success, config: %#x\n", str[op],
                config);
    else
        ERR_MSG(" %s Quad failed! reg: %#x\n", str[op], config);

    MTD_PR(QE_DBG, "\t* End SPI Nor %s Quad.\n", str[op]);

    return op;
}

/*
 *   enable 4byte adress for SPI which memory more than 16M
 *   */
int spinor_general_entry_4addr(struct spi *spi, int enable)
{
    const char *str[] = {"Disable", "Enable"};
    struct spinor_host *host = (struct spinor_host *)spi->host;
    int reg;

    MTD_PR(AC_DBG, "\t* Start SPI Nor flash %s 4-byte mode.\n",
            str[enable]);

    if (spi->addrcycle != SPI_4BYTE_ADDR_LEN) {
        MTD_PR(AC_DBG, "\t* Flash isn't support entry 4-byte mode.\n");
        return 0;
    }

    reg = hifmc100_read_reg(spi, SPI_CMD_RDSR3);
    MTD_PR(AC_DBG, "\t  Read Status Register-3[%#x]:%#x\n", SPI_CMD_RDSR3,
            reg);
    if (SPI_NOR_GET_4BYTE_BY_CR(reg) == enable) {
        MTD_PR(AC_DBG, "\t* 4-byte was %sd, reg:%#x\n", str[enable],
                reg);
        return 0;
    }

    if (enable)
        reg = SPI_CMD_EN4B;
    else
        reg = SPI_CMD_EX4B;
    reg = FMC_CMD_CMD1(reg);
    reg_write(host, reg, FMC_CMD);

    reg = OP_CFG_FM_CS(spi->cs);
    reg_write(host, reg, FMC_OP_CFG);

    reg = FMC_OP_CMD1_EN(ENABLE) | FMC_OP_REG_OP_START;
    reg_write(host, reg, FMC_OP);

    FMC_CMD_WAIT_CPU_FINISH(host);

    host->set_host_addr_mode(host, enable);

    MTD_PR(AC_DBG, "\t* End SPI Nor flash %s 4-byte mode.\n", str[enable]);
    return 0;
}

/*
    Read status[C0H]:[0]bit OIP, judge whether the device is busy or not
*/
int spinor_general_wait_ready(struct spi *spi)
{
    unsigned char status;

    do {
        status = hifmc100_read_reg(spi, SPI_CMD_RDSR);
        if (!(status & SPI_NOR_SR_WIP_MASK))
            return 0;
    } while (1);

    return 1;
}

/*
 *   configure prepared for dma or bus read or write mode
 *   */
int spinor_general_bus_prepare(struct spi *spi, int op)
{
    return 0;
}

