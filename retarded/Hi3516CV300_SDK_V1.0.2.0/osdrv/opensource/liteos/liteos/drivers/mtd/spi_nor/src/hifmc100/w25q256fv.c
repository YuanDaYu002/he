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
#define W25Q256FV_CR_4BYTE_MASK        0x1

#define WB_SPI_NOR_SR_ADS_MASK    1
#define WB_SPI_NOR_GET_4BYTE_BY_SR(sr)    ((sr) & WB_SPI_NOR_SR_ADS_MASK)

#define SPI_CMD_FIRST_RESET_4ADDR  (0x66)
#define SPI_CMD_SECOND_RESET_4ADDR (0x99)

extern u_char hifmc100_read_reg(struct spi *spi, u_char cmd);
/****************************************************************************/
int spi_w25q256fv_entry_4addr(struct spi *spi, int enable)
{
    unsigned char status;
    unsigned int reg;
    const char *str[] = {"Disable", "Enable"};
    struct spinor_host *host = (struct spinor_host *)spi->host;

    MTD_PR(AC_DBG, "\t* Start W25Q256FV SPI Nor %s 4-byte mode.\n",
            str[enable]);

    if (spi->addrcycle != SPI_4BYTE_ADDR_LEN) {
        MTD_PR(AC_DBG, "\t* W25Q(128/256)FV not support 4B mode.\n");
        return 0;
    }

    status = hifmc100_read_reg(spi, SPI_CMD_RDSR3);
    MTD_PR(AC_DBG, "\t  Read Status Register-3[%#x]:%#x\n", SPI_CMD_RDSR3,
            status);
    if (WB_SPI_NOR_GET_4BYTE_BY_SR(status) == enable) {
        MTD_PR(AC_DBG, "\t* 4-byte was %sd, reg:%#x\n", str[enable],
                status);
        return 0;
    }

    /* This chip should not write enable here,
     * we have confirmed with the WINBOND */
    /* spi->driver->write_enable(spi); */

    if (enable) {
        reg = FMC_CMD_CMD1(SPI_CMD_EN4B);
        reg_write(host, reg, FMC_CMD);

        reg = OP_CFG_FM_CS(spi->cs);
        reg_write(host, reg, FMC_OP_CFG);

        reg = FMC_OP_CMD1_EN(ENABLE) | FMC_OP_REG_OP_START;
        reg_write(host, reg, FMC_OP);

        FMC_CMD_WAIT_CPU_FINISH(host);

        status = hifmc100_read_reg(spi, SPI_CMD_RDSR3);
        MTD_PR(AC_DBG, "\t  Get Status Register 3[%#x]:%#x\n",
                SPI_CMD_RDSR3, status);
        if (status & W25Q256FV_CR_4BYTE_MASK)
            MTD_PR(AC_DBG, "\t  Enter 4-byte success, reg[%#x]\n",
                    status);
        else
            ERR_MSG(" Enter 4-byte failed! [%#x]\n", status);

    } else {
/* reset cmd */
        reg = FMC_CMD_CMD1(SPI_CMD_FIRST_RESET_4ADDR);
        reg_write(host, reg, FMC_CMD);

        reg = OP_CFG_FM_CS(spi->cs);
        reg_write(host, reg, FMC_OP_CFG);

        reg = FMC_OP_CMD1_EN(ENABLE) | FMC_OP_REG_OP_START;
        reg_write(host, reg, FMC_OP);

        FMC_CMD_WAIT_CPU_FINISH(host);

        reg = FMC_CMD_CMD1(SPI_CMD_SECOND_RESET_4ADDR);
        reg_write(host, reg, FMC_CMD);

        reg = OP_CFG_FM_CS(spi->cs);
        reg_write(host, reg, FMC_OP_CFG);

        reg = FMC_OP_CMD1_EN(ENABLE) | FMC_OP_REG_OP_START;
        reg_write(host, reg, FMC_OP);

        FMC_CMD_WAIT_CPU_FINISH(host);

        MTD_PR(AC_DBG, "\tnow W25Q256FV start software reset\n");
    }

    host->set_host_addr_mode(host, enable);

    MTD_PR(AC_DBG, "\t* End W25Q256FV enter 4-byte mode.\n");

    return 0;
}

/*
   enable QE bit if QUAD read write is supported by W25Q(128/256)FV
*/
int spi_w25q256fv_qe_enable(struct spi *spi)
{
    unsigned char status, op;
    unsigned int reg;
    const char *str[] = {"Disable", "Enable"};
    struct spinor_host *host = (struct spinor_host *)spi->host;

    op = spi_is_quad(spi);

    MTD_PR(QE_DBG, "\t* Start SPI Nor W25Q(128/256)FV %s Quad.\n", str[op]);

    status = hifmc100_read_reg(spi, SPI_CMD_RDSR2);
    MTD_PR(QE_DBG, "\t  Read Status Register-2[%#x]%#x\n", SPI_CMD_RDSR2,
            status);
    if (SPI_NOR_GET_QE_BY_CR(status) == op) {
        MTD_PR(QE_DBG, "\t* Quad was %s status:%#x\n", str[op], status);
        return op;
    }

    spi->driver->write_enable(spi);

    if (op)
        status |= SPI_NOR_CR_QE_MASK;
    else
        status &= ~SPI_NOR_CR_QE_MASK;
    writeb(status, host->membase);
    MTD_PR(QE_DBG, "\t  Write IO[%#x]%#x\n", (unsigned int)host->membase,
            *(unsigned char *)host->membase);

    /* There is new cmd for Write Status Register 2 by W25Q(128/256)FV */
    reg = FMC_CMD_CMD1(SPI_CMD_WRSR2);
    reg_write(host, reg, FMC_CMD);

    reg = OP_CFG_FM_CS(spi->cs);
    reg_write(host, reg, FMC_OP_CFG);

    reg = FMC_DATA_NUM_CNT(SPI_NOR_SR_LEN);
    reg_write(host, reg, FMC_DATA_NUM);

    reg = FMC_OP_CMD1_EN(ENABLE)
        | FMC_OP_WRITE_DATA_EN(ENABLE)
        | FMC_OP_REG_OP_START;
    reg_write(host, reg, FMC_OP);

    FMC_CMD_WAIT_CPU_FINISH(host);

    MTD_PR(QE_DBG, "\t* End SPI Nor W25Q(128/256)FV %s Quad.\n", str[op]);

    return op;
}

