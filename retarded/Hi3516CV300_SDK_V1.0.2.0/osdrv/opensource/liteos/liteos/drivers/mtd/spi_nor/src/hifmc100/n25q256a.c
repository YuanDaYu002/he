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

int spi_n25q256a_entry_4addr(struct spi *spi, int enable)
{
    unsigned int regval;
    struct spinor_host *host = (struct spinor_host *)spi->host;
    if (spi->addrcycle != SPI_4BYTE_ADDR_LEN) {
        MTD_PR(AC_DBG, "\t* Flash isn't support 4-byte mode.\n");
        return 0;
    }

    spi->driver->write_enable(spi);

    if (enable) {
        regval = FMC_CMD_CMD1(SPI_CMD_EN4B);
        reg_write(host, regval, FMC_CMD);
        MTD_PR(AC_DBG, "\t  Set CMD[%#x]%#x\n", FMC_CMD, regval);
        MTD_PR(AC_DBG, "now is 4-byte address mode\n");
    } else {
        regval = FMC_CMD_CMD1(SPI_CMD_EX4B);
        reg_write(host, regval, FMC_CMD);
        MTD_PR(AC_DBG, "\t  Set CMD[%#x]%#x\n", FMC_CMD, regval);
        MTD_PR(AC_DBG, "now is 3-byte address mode\n");
    }


    regval = OP_CFG_FM_CS(spi->cs);
    reg_write(host, regval, FMC_OP_CFG);
    MTD_PR(AC_DBG, "\t  Set OP_CFG[%#x]%#x\n", FMC_OP_CFG, regval);

    regval = FMC_OP_CMD1_EN(ENABLE) | FMC_OP_REG_OP_START;
    reg_write(host, regval, FMC_OP);
    MTD_PR(AC_DBG, "\t  Set OP[%#x]%#x\n", FMC_OP, regval);

    FMC_CMD_WAIT_CPU_FINISH(host);

    host->set_host_addr_mode(host, enable);

    return 0;
}

int spi_n25q256a_qe_enable(struct spi *spi)
{
    return 0;
}

