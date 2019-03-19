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

#define SPI_CMD_FIRST_RESET_4ADDR  (0x66)
#define SPI_CMD_SECOND_RESET_4ADDR (0x99)

int spi_w25q256fv_entry_4addr(struct spi *spi, int enable)
{
    struct spinor_host *host = (struct spinor_host *)spi->host;
    int reg;

    if (spi->addrcycle != SPI_4BYTE_ADDR_LEN)
        return 0;
    /* This chip should not enable write here,
     * we have confirmed with the WINBOND */
    /* spi->driver->write_enable(spi); */
    if (enable) {
        reg = SPI_CMD_EN4B;
        reg_write(host, reg, HISFC350_CMD_INS);

        reg = HISFC350_CMD_CONFIG_SEL_CS(spi->cs)
            | HISFC350_CMD_CONFIG_START;
        reg_write(host, reg, HISFC350_CMD_CONFIG);

        HISFC350_CMD_WAIT_CPU_FINISH(host);
    } else {
        reg = SPI_CMD_FIRST_RESET_4ADDR;
        reg_write(host, reg, HISFC350_CMD_INS);

        reg = HISFC350_CMD_CONFIG_SEL_CS(spi->cs)
            | HISFC350_CMD_CONFIG_START;
        reg_write(host, reg, HISFC350_CMD_CONFIG);


        HISFC350_CMD_WAIT_CPU_FINISH(host);

        reg = SPI_CMD_SECOND_RESET_4ADDR;
        reg_write(host, reg, HISFC350_CMD_INS);

        reg = HISFC350_CMD_CONFIG_SEL_CS(spi->cs)
                | HISFC350_CMD_CONFIG_START;
        reg_write(host, reg, HISFC350_CMD_CONFIG);

        HISFC350_CMD_WAIT_CPU_FINISH(host);
    }
    host->set_host_addr_mode(host, enable);

    return 0;
}

int spi_w25q256fv_qe_enable(struct spi *spi)
{
    return spinor_general_qe_enable(spi);
}
