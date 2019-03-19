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

#define GD_SPI_CMD_SR_DISQE     (0x0)
#define SPI_CMD_WRSR1           (0x1)
#define GD_SPI_CMD_SR_QE        (0x2)

int spi_gd25qxxx_qe_enable(struct spi *spi)
{
    struct spinor_host *host = (struct spinor_host *)spi->host;
    unsigned int reg = 0;
    unsigned int qe_op1 = 0;
    unsigned int qe_op2 = 0;

    if (hisfc350_spi_is_quad(spi)) {
        qe_op1 = SPI_CMD_SR_QE;
        qe_op2 = GD_SPI_CMD_SR_QE;
    } else {
        qe_op1 = SPI_CMD_SR_XQE;
        qe_op2 = GD_SPI_CMD_SR_DISQE;
    }

    spi->driver->write_enable(spi);

    /* First, we enable QE(4bit r&w) for 16pin gd flash */
    reg = SPI_CMD_WRSR1;
    reg_write(host, reg, HISFC350_CMD_INS);

    reg = qe_op1;
    reg_write(host, reg, HISFC350_CMD_DATABUF0);

    reg = HISFC350_CMD_CONFIG_MEM_IF_TYPE(spi->
                write->iftype)
            | HISFC350_CMD_CONFIG_DATA_CNT(2)
            | HISFC350_CMD_CONFIG_DATA_EN
            | HISFC350_CMD_CONFIG_DUMMY_CNT(spi->
                write->dummy)
            | HISFC350_CMD_CONFIG_SEL_CS(spi->cs)
            | HISFC350_CMD_CONFIG_START;
    reg_write(host, reg, HISFC350_CMD_CONFIG);

    HISFC350_CMD_WAIT_CPU_FINISH(host);

    spi->driver->wait_ready(spi);

    /* Second, we enable QE for 8 pin gd flash. This will not affect
       16pin gd spi, if the QE bit has been set 1.
     */
    spi->driver->write_enable(spi);

    reg = SPI_CMD_WRSR2;
    reg_write(host, reg, HISFC350_CMD_INS);

    reg = qe_op2;
    reg_write(host, reg, HISFC350_CMD_DATABUF0);

    reg_write(host, reg, HISFC350_CMD_CONFIG);
    reg = HISFC350_CMD_CONFIG_MEM_IF_TYPE(spi->
                write->iftype)
            | HISFC350_CMD_CONFIG_DATA_CNT(1)
            | HISFC350_CMD_CONFIG_DATA_EN
            | HISFC350_CMD_CONFIG_DUMMY_CNT(spi->
                write->dummy)
            | HISFC350_CMD_CONFIG_SEL_CS(spi->cs)
            | HISFC350_CMD_CONFIG_START;
    reg_write(host, reg, HISFC350_CMD_CONFIG);

    HISFC350_CMD_WAIT_CPU_FINISH(host);

    spi->driver->wait_ready(spi);

    return 0;
}

