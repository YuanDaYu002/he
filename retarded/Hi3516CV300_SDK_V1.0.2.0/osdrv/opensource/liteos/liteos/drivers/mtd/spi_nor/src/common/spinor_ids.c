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

#include "string.h"

#include "mtd_common.h"
#include "spi_common.h"
#include "spinor_ids.h"

#define CONFIG_CLOSE_SPI_8PIN_4IO

/*****************************************************************************/
SET_READ_STD(0, INFINITE, 0);
SET_READ_STD(0, INFINITE, 20);
SET_READ_STD(0, INFINITE, 32);
SET_READ_STD(0, INFINITE, 33);
SET_READ_STD(0, INFINITE, 40);
SET_READ_STD(0, INFINITE, 50);
SET_READ_STD(0, INFINITE, 54);
SET_READ_STD(0, INFINITE, 55);
SET_READ_STD(0, INFINITE, 66);
SET_READ_STD(0, INFINITE, 80);

SET_READ_FAST(1, INFINITE, 50);
SET_READ_FAST(1, INFINITE, 64);
SET_READ_FAST(1, INFINITE, 66);
SET_READ_FAST(1, INFINITE, 75);
SET_READ_FAST(1, INFINITE, 80);
SET_READ_FAST(1, INFINITE, 86);
SET_READ_FAST(1, INFINITE, 100);
SET_READ_FAST(1, INFINITE, 104);
SET_READ_FAST(1, INFINITE, 108);
SET_READ_FAST(1, INFINITE, 133);

SET_READ_DUAL(1, INFINITE, 64);
SET_READ_DUAL(1, INFINITE, 75);
SET_READ_DUAL(1, INFINITE, 80);
SET_READ_DUAL(1, INFINITE, 84);
SET_READ_DUAL(1, INFINITE, 104);
SET_READ_DUAL(2, INFINITE, 104);
SET_READ_DUAL(1, INFINITE, 108);
SET_READ_DUAL(1, INFINITE, 133);

SET_READ_DUAL_ADDR(2, INFINITE, 64);
SET_READ_DUAL_ADDR(0, INFINITE, 80);
SET_READ_DUAL_ADDR(1, INFINITE, 80);
SET_READ_DUAL_ADDR(1, INFINITE, 84);
SET_READ_DUAL_ADDR(2, INFINITE, 84);
SET_READ_DUAL_ADDR(1, INFINITE, 104);
SET_READ_DUAL_ADDR(1, INFINITE, 108);
SET_READ_DUAL_ADDR(1, INFINITE, 133);
SET_READ_DUAL_ADDR(2, INFINITE, 133);
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
SET_READ_QUAD(1, INFINITE, 64);
SET_READ_QUAD(1, INFINITE, 80);
SET_READ_QUAD(1, INFINITE, 84);
SET_READ_QUAD(1, INFINITE, 104);
SET_READ_QUAD(1, INFINITE, 108);
SET_READ_QUAD(1, INFINITE, 133);

SET_READ_QUAD_ADDR(2, INFINITE, 80);
/* SET_READ_QUAD_ADDR(3, INFINITE, 50); */
SET_READ_QUAD_ADDR(3, INFINITE, 75);
SET_READ_QUAD_ADDR(3, INFINITE, 80);
SET_READ_QUAD_ADDR(5, INFINITE, 64);
SET_READ_QUAD_ADDR(5, INFINITE, 84);
SET_READ_QUAD_ADDR(3, INFINITE, 104);
SET_READ_QUAD_ADDR(3, INFINITE, 108);
SET_READ_QUAD_ADDR(5, INFINITE, 125);
SET_READ_QUAD_ADDR(3, INFINITE, 133);
#endif
/*****************************************************************************/
SET_WRITE_STD(0, 256, 0);
SET_WRITE_STD(0, 256, 33);
SET_WRITE_STD(0, 256, 50);
SET_WRITE_STD(0, 256, 64);
SET_WRITE_STD(0, 256, 66);
SET_WRITE_STD(0, 256, 75);
SET_WRITE_STD(0, 256, 80);
SET_WRITE_STD(0, 256, 86);
SET_WRITE_STD(0, 256, 100);
SET_WRITE_STD(0, 256, 104);
SET_WRITE_STD(0, 256, 108);
SET_WRITE_STD(0, 256, 133);

SET_WRITE_DUAL(0, 256, 64);
SET_WRITE_DUAL(0, 256, 75);
SET_WRITE_DUAL(0, 256, 108);
SET_WRITE_DUAL(0, 256, 133);

SET_WRITE_DUAL_ADDR(0, 256, 64);
SET_WRITE_DUAL_ADDR(0, 256, 75);
SET_WRITE_DUAL_ADDR(0, 256, 108);

SET_WRITE_DUAL_ADDR(0, 256, 133);
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
SET_WRITE_QUAD(0, 256, 64);
SET_WRITE_QUAD(0, 256, 80);
SET_WRITE_QUAD(0, 256, 108);
SET_WRITE_QUAD(0, 256, 133);

SET_WRITE_QUAD_ADDR(0, 256, 33);
SET_WRITE_QUAD_ADDR(0, 256, 80);
SET_WRITE_QUAD_ADDR(0, 256, 104);
SET_WRITE_QUAD_ADDR(0, 256, 133);
#endif
/*****************************************************************************/
SET_ERASE_SECTOR_32K(0, _32K, 0);

SET_ERASE_SECTOR_64K(0, _64K, 0);
SET_ERASE_SECTOR_64K(0, _64K, 33);
SET_ERASE_SECTOR_64K(0, _64K, 50);
SET_ERASE_SECTOR_64K(0, _64K, 64);
SET_ERASE_SECTOR_64K(0, _64K, 66);
SET_ERASE_SECTOR_64K(0, _64K, 75);
SET_ERASE_SECTOR_64K(0, _64K, 80);
SET_ERASE_SECTOR_64K(0, _64K, 86);
SET_ERASE_SECTOR_64K(0, _64K, 100);
SET_ERASE_SECTOR_64K(0, _64K, 104);
SET_ERASE_SECTOR_64K(0, _64K, 108);
SET_ERASE_SECTOR_64K(0, _64K, 133);

SET_ERASE_SECTOR_256K(0, _256K, 50);
SET_ERASE_SECTOR_256K(0, _256K, 104);
/*****************************************************************************/

extern int spinor_general_wait_ready(struct spi *spi);
extern int spinor_general_write_enable(struct spi *spi);
extern int spinor_general_qe_enable(struct spi *spi);
extern int spinor_general_entry_4addr(struct spi *spi, int enable);
extern int spinor_general_bus_prepare(struct spi *spi, int op);

static struct spi_drv spi_driver_general = {
    .wait_ready = spinor_general_wait_ready,
    .write_enable = spinor_general_write_enable,
    .entry_4addr = spinor_general_entry_4addr,
    .bus_prepare = spinor_general_bus_prepare,
    .qe_enable = spinor_general_qe_enable,
};

extern int spinor_qe_not_enable(struct spi *spi);
static struct spi_drv spi_driver_no_qe = {
    .wait_ready = spinor_general_wait_ready,
    .write_enable = spinor_general_write_enable,
    .entry_4addr = spinor_general_entry_4addr,
    .bus_prepare = spinor_general_bus_prepare,
    .qe_enable = spinor_qe_not_enable,
};

extern int spi_s25fl256s_entry_4addr(struct spi *spi, int enable);
static struct spi_drv spi_driver_s25fl256s = {
    .wait_ready = spinor_general_wait_ready,
    .write_enable = spinor_general_write_enable,
    .entry_4addr = spi_s25fl256s_entry_4addr,
    .bus_prepare = spinor_general_bus_prepare,
    .qe_enable = spinor_general_qe_enable,
};

extern int spi_w25q256fv_entry_4addr(struct spi *spi, int enable);
extern int spi_w25q256fv_qe_enable(struct spi *spi);
static struct spi_drv spi_driver_w25q256fv = {
    .wait_ready = spinor_general_wait_ready,
    .write_enable = spinor_general_write_enable,
    .entry_4addr = spi_w25q256fv_entry_4addr,
    .bus_prepare = spinor_general_bus_prepare,
    .qe_enable = spi_w25q256fv_qe_enable,
};

extern int spi_mx25l25635e_qe_enable(struct spi *spi);
static struct spi_drv spi_driver_mx25l25635e = {
    .wait_ready = spinor_general_wait_ready,
    .write_enable = spinor_general_write_enable,
    .entry_4addr = spinor_general_entry_4addr,
    .bus_prepare = spinor_general_bus_prepare,
    .qe_enable = spi_mx25l25635e_qe_enable,
};

extern int spi_mx25l25635e_qe_enable(struct spi *spi);
static struct spi_drv spi_driver_f25l64q = {
    .wait_ready = spinor_general_wait_ready,
    .write_enable = spinor_general_write_enable,
    .entry_4addr = spinor_general_entry_4addr,
    .bus_prepare = spinor_general_bus_prepare,
    .qe_enable = spi_mx25l25635e_qe_enable,
};


extern int spi_gd25qxxx_qe_enable(struct spi *spi);
static struct spi_drv spi_driver_gd25qxxx = {
    .wait_ready = spinor_general_wait_ready,
    .write_enable = spinor_general_write_enable,
    .entry_4addr = spinor_general_entry_4addr,
    .bus_prepare = spinor_general_bus_prepare,
    .qe_enable = spi_gd25qxxx_qe_enable,
};

extern int spi_n25q256a_entry_4addr(struct spi *spi, int enable);
extern int spi_n25q256a_qe_enable(struct spi *spi);
static struct spi_drv  spi_driver_n25q256a = {
    .wait_ready   = spinor_general_wait_ready,
    .write_enable = spinor_general_write_enable,
    .entry_4addr  = spi_n25q256a_entry_4addr,
    .qe_enable = spi_n25q256a_qe_enable,
};

/*****************************************************************************/
static struct spi_nor_info spi_nor_info_table[] = {
    /* name        id    id_len    chipsize(Bytes)    erasesize */
    {
        "at25fs010", {0x1f, 0x66, 0x01}, 3, _128K, _32K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },
        {
            &WRITE_STD(0, 256, 0),
            0
        },
        {
            &ERASE_SECTOR_32K(0, _32K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "at25fs040",  {0x1f, 0x66, 0x04}, 3,  _512K,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },
        {
            &WRITE_STD(0, 256, 0),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "at25df041a", {0x1f, 0x44, 0x01}, 3,  _512K,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },
        {
            &WRITE_STD(0, 256, 0),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "at25df641",  {0x1f, 0x48, 0x00}, 3,  _8M,    _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },
        {
            &WRITE_STD(0, 256, 0),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "at26f004",   {0x1f, 0x04, 0x00}, 3,  _512K,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },
        {
            &WRITE_STD(0, 256, 0),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "at26df081a", {0x1f, 0x45, 0x01}, 3,  _1M,    _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },
        {
            &WRITE_STD(0, 256, 0),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "at26df161a", {0x1f, 0x46, 0x01}, 3,  _2M,    _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },
        {
            &WRITE_STD(0, 256, 0),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "at26df321",  {0x1f, 0x47, 0x01}, 3,  _4M,    _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },
        {
            &WRITE_STD(0, 256, 0),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    /* Macronix */
    {
        "mx25l4005a",  {0xc2, 0x20, 0x13}, 3, _512K,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },
        {
            &WRITE_STD(0, 256, 0),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "MX25L8006E",  {0xc2, 0x20, 0x14}, 3, _1M,    _64K, 3,
        {
            &READ_STD(0, INFINITE, 33),
            &READ_FAST(1, INFINITE, 86),
            &READ_DUAL(1, INFINITE, 80),
            0
        },
        {
            &WRITE_STD(0, 256, 86),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 86),
            0
        },
        &spi_driver_general,
    },

    {
        "MX25L1606E",  {0xc2, 0x20, 0x15}, 3, _2M,    _64K, 3,
        {
            &READ_STD(0, INFINITE, 33),
            &READ_FAST(1, INFINITE, 86),
            &READ_DUAL(1, INFINITE, 80),
            0
        },
        {
            &WRITE_STD(0, 256, 86),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 86),
            0
        },
        &spi_driver_no_qe,
    },

    {
        "mx25l3205d",  {0xc2, 0x20, 0x16}, 3, _4M,    _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },
        {
            &WRITE_STD(0, 256, 0),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
		"MX25L6436F",  {0xc2, 0x20, 0x17}, 3, _8M,    _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
			&READ_FAST(1, INFINITE, 133),
			&READ_DUAL(1, INFINITE, 133),
			&READ_DUAL_ADDR(1, INFINITE, 133),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&READ_QUAD(1, INFINITE, 133),
			&READ_QUAD_ADDR(3, INFINITE, 133),
#endif
            0
        },

        {
			&WRITE_STD(0, 256, 133),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&WRITE_QUAD_ADDR(0, 256, 133),
#endif
            0
        },

        {
			&ERASE_SECTOR_64K(0, _64K, 133),
            0
        },
        &spi_driver_mx25l25635e,
    },

	{
		"MX25V1635F",  {0xc2, 0x23, 0x15}, 3, _2M,    _64K, 3,
		{
			&READ_STD(0, INFINITE, 33),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 80),
			&READ_DUAL_ADDR(1, INFINITE, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&READ_QUAD(1, INFINITE, 80),
			&READ_QUAD_ADDR(3, INFINITE, 80),
#endif
			0
		},
		{
			&WRITE_STD(0, 256, 33),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&WRITE_QUAD_ADDR(0, 256, 80),
#endif
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 80),
			0
		},
		&spi_driver_mx25l25635e,
	},
    /* MX25R6435F Wide Voltage Range 1.65~3.6V */
    {
        "MX25R6435F", {0xc2, 0x28, 0x17}, 3, _8M, _64K, 3,
        {
            &READ_STD(0, INFINITE, 33),
            &READ_FAST(1, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 80),
            &READ_DUAL_ADDR(1, INFINITE, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, 80),
            &READ_QUAD_ADDR(3, INFINITE, 80),
#endif
            0
        },

        {
			&WRITE_STD(0, 256, 33),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&WRITE_QUAD_ADDR(0, 256, 33),
#endif
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 33),
			0
		},
		&spi_driver_mx25l25635e,
	},
	{
		"MX25U6435F", {0xc2, 0x25, 0x37}, 3, _8M, _64K, 3,
		{
			&READ_STD(0, INFINITE, 50),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 84),
			&READ_DUAL_ADDR(1, INFINITE, 84),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&READ_QUAD(1, INFINITE, 80),
			&READ_QUAD_ADDR(3, INFINITE, 80),
#endif
			0
		},
		{
            &WRITE_STD(0, 256, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&WRITE_QUAD_ADDR(0, 256, 80),
#endif
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 80),
			0
		},
		&spi_driver_mx25l25635e,
	},
	{
		"MX25U12835F", {0xc2, 0x25, 0x38}, 3, _16M, _64K, 3,
		{
			&READ_STD(0, INFINITE, 55),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 84),
			&READ_DUAL_ADDR(1, INFINITE, 84),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&READ_QUAD(1, INFINITE, 80),
			&READ_QUAD_ADDR(3, INFINITE, 80),
#endif
			0
		},
		{
			&WRITE_STD(0, 256, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&WRITE_QUAD_ADDR(0, 256, 80),
#endif
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 80),
            0
        },
        &spi_driver_mx25l25635e,
    },

    {
		"MX25L128XX", {0xc2, 0x20, 0x18}, 3, _16M, _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 104),
			&READ_DUAL(1, INFINITE, 104),
			&READ_DUAL_ADDR(1, INFINITE, 104),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, 104),
            &READ_QUAD_ADDR(3, INFINITE, 104),
#endif
            0
        },

        {
            &WRITE_STD(0, 256, 104),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&WRITE_QUAD_ADDR(0, 256, 104),
#endif
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_mx25l25635e,
    },
    /* MX25U25635F, 1.65-2.0V */
    {
        "MX25U25635F", {0xc2, 0x25, 0x39}, 3, _32M, _64K, 4,
        {
            &READ_STD(0, INFINITE, 55),
            &READ_FAST(1, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 84),
            &READ_DUAL_ADDR(1, INFINITE, 84),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, 80),
            &READ_QUAD_ADDR(3, INFINITE, 80),
#endif
            0
        },

        {
            &WRITE_STD(0, 256, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &WRITE_QUAD_ADDR(0, 256, 80),
#endif
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 80),
            0
        },
        &spi_driver_mx25l25635e,
    },

    {
		"MX25L(256/257)XX",
		{0xc2, 0x20, 0x19}, 3, _32M, _64K, 4,
        {
			&READ_STD(0, INFINITE, 40/*50*/),
            &READ_FAST(1, INFINITE, 104),
			&READ_DUAL(2, INFINITE, 104),
            &READ_DUAL_ADDR(1, INFINITE, 84),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&READ_QUAD_ADDR(3, INFINITE, 75),
#endif
            0
        },

        {
			&WRITE_STD(0, 256, 75),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&WRITE_QUAD_ADDR(0, 256, 104),
#endif
            0
        },

        {
			&ERASE_SECTOR_64K(0, _64K, 80),
            0
        },
        &spi_driver_mx25l25635e,
    },

    {
        "mx25l1655d",  {0xc2, 0x26, 0x15}, 3, _2M,    _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },
        {
            &WRITE_STD(0, 256, 0),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "mx25l12855e", {0xc2, 0x26, 0x18}, 3, _16M,   _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "s25sl004a", {0x01, 0x02, 0x12}, 3, (_64K * 8),   _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "s25sl008a", {0x01, 0x02, 0x13}, 3, (_64K * 16),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "s25sl016a", {0x01, 0x02, 0x14}, 3, (_64K * 32),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "S25FL064P", {0x01, 0x02, 0x16, 0x4d}, 4, (_64K * 128), _64K, 3,
        {
            &READ_STD(0, INFINITE, 40),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, 80),
#endif
            0
        },

        {
            &WRITE_STD(0, 256, 104),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &WRITE_QUAD(0, 256, 80),
#endif
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_general,
    },

    {
        "s25sl064a", {0x01, 0x02, 0x16}, 3, (_64K * 128), _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },
    /* Spansion */

    {
        "S25FL032P", {0x01, 0x02, 0x15, 0x4d}, 4, (_64K * 64),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 40),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 80),
            &READ_DUAL_ADDR(0, INFINITE, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, 80),
            &READ_QUAD_ADDR(2, INFINITE, 80),
#endif
            0
        },

        {
            &WRITE_STD(0, 256, 104),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &WRITE_QUAD(0, 256, 80),
#endif
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_general,
    },

    {
        "S25FL032A", {0x01, 0x02, 0x15}, 3, (_64K * 64),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 33),
            &READ_FAST(1, INFINITE, 50),
            0
        },

        {
            &WRITE_STD(0, 256, 50),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 50),
            0
        },
        &spi_driver_general,
    },

    {
        "S25FL128P-0",
        {0x01, 0x20, 0x18, 0x03, 0x00}, 5, (_256K * 64),  _256K, 3,
        {
            &READ_STD(0, INFINITE, 40),
            &READ_FAST(1, INFINITE, 104),
            0
        },

        {
            &WRITE_STD(0, 256, 104),
            0
        },

        {
            &ERASE_SECTOR_256K(0, _256K, 104),
            0
        },
        &spi_driver_no_qe,
    },

    {
        "S25FL128P-1",
        {0x01, 0x20, 0x18, 0x03, 0x01}, 5, (_64K * 256),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 40),
            &READ_FAST(1, INFINITE, 104),
            0
        },

        {
            &WRITE_STD(0, 256, 104), 0},

        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_no_qe,
    },

    {
        "S25FL129P0",
        {0x01, 0x20, 0x18, 0x4d, 0x00}, 5, (_256K * 64),  _256K, 3,
        {
            &READ_STD(0, INFINITE, 40),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 80),
            &READ_DUAL_ADDR(0, INFINITE, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, 80),
            &READ_QUAD_ADDR(2, INFINITE, 80),
#endif
            0
        },

        {
            &WRITE_STD(0, 256, 104),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &WRITE_QUAD(0, 256, 80),
#endif
            0
        },

        {
            &ERASE_SECTOR_256K(0, _256K, 104),
            0
        },
        &spi_driver_general,
    },

    {
        "S25FL129P1",
        {0x01, 0x20, 0x18, 0x4d, 0x01}, 5, (_64K * 256),  _64K,  3,
        {
            &READ_STD(0, INFINITE, 40),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 64),
            &READ_DUAL_ADDR(1, INFINITE, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, 80),
            &READ_QUAD_ADDR(3, INFINITE, 80),
#endif
            0
        },

        {
            &WRITE_STD(0, 256, 104),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &WRITE_QUAD(0, 256, 80),
#endif
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_general,
    },

    {
        "S25FL256S", {0x01, 0x02, 0x19, 0x4d, 0x01}, 5, _32M,  _64K,  4,
        {
            &READ_STD(0, INFINITE, 40),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 64),
            &READ_DUAL_ADDR(1, INFINITE, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, 80),
            &READ_QUAD_ADDR(3, INFINITE, 80),
#endif
            0
        },

        {
            &WRITE_STD(0, 256, 104),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &WRITE_QUAD(0, 256, 80),
#endif
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_s25fl256s,
    },

    /*
    The chip and chip W25Q16B have the same chipid,
    but clock frequency have some difference

    {"S25FL016K", {0xef, 0x40, 0x15}, 3, (_64K * 32),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 104),
            &READ_QUAD(1, INFINITE, 104),
            0
        },
        {
            &WRITE_STD(0, 256, 104),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        }
    },
    */

    /* SST -- large erase sizes are "overlays", "sectors" are 4K */
    {
        "sst25vf040b", {0xbf, 0x25, 0x8d}, 3, (_64K * 8),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "sst25vf080b", {0xbf, 0x25, 0x8e}, 3, (_64K * 16), _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "sst25vf016b", {0xbf, 0x25, 0x41}, 3, (_64K * 32), _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },
        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "sst25vf032b", {0xbf, 0x25, 0x4a}, 3, (_64K * 64), _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "sst25wf512",  {0xbf, 0x25, 0x01}, 3, (_64K * 1),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "sst25wf010",  {0xbf, 0x25, 0x02}, 3, (_64K * 2),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "sst25wf020",  {0xbf, 0x25, 0x03}, 3, (_64K * 4),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "sst25wf040",  {0xbf, 0x25, 0x04}, 3, (_64K * 8),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    /* ST Microelectronics -- newer production may have feature updates */
    {
        "m25p05",  {0x20, 0x20, 0x10}, 3, (_32K * 2), _32K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_32K(0, _32K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "m25p10",  {0x20, 0x20, 0x11}, 3, (_32K * 4), _32K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_32K(0, _32K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "m25p20",  {0x20, 0x20, 0x12}, 3, (_64K * 4),   _64K,  3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "m25p40",  {0x20, 0x20, 0x13}, 3, (_64K * 8),   _64K,  3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "m25p80",  {0x20, 0x20, 0x14}, 3, (_64K * 16),  _64K,  3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "m25p16",  {0x20, 0x20, 0x15}, 3, (_64K * 32),  _64K,  3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "M25P32",  {0x20, 0x20, 0x16, 0x10}, 4, _4M, _64K, 3,
        {
            &READ_STD(0, INFINITE, 33),
            &READ_FAST(1, INFINITE, 75),
            0
        },

        {
            &WRITE_STD(0, 256, 75),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 75),
            0
        },
        &spi_driver_general,
    },

    {
        "m25p64",  {0x20, 0x20, 0x17}, 3, (_64K * 128), _64K,  3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "M25P128", {0x20, 0x20, 0x18}, 3, _16M, _256K, 3,
        {
            &READ_STD(0, INFINITE, 20),
            &READ_FAST(1, INFINITE, 50),
            0
        },

        {
            &WRITE_STD(0, 256, 50),
            0
        },

        {
            &ERASE_SECTOR_256K(0, _256K, 50),
            0
        },
        &spi_driver_general,
    },

    {
        "m45pe10", {0x20, 0x40, 0x11}, 3, (_64K * 2),   _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "m45pe80", {0x20, 0x40, 0x14}, 3, (_64K * 16),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "m45pe16", {0x20, 0x40, 0x15}, 3, (_64K * 32),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "m25pe80", {0x20, 0x80, 0x14}, 3, (_64K * 16), _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "m25pe16", {0x20, 0x80, 0x15}, 3, (_64K * 32), _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "N25Q032", {0x20, 0xba, 0x16}, 3, (_64K * 64), _64K, 3,
        {
            &READ_STD(0, INFINITE, 32/*54*/),
            &READ_FAST(1, INFINITE, 64/*108*/),
            &READ_DUAL(1, INFINITE, 64/*108*/),
            &READ_DUAL_ADDR(2, INFINITE, 64/*108*/),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, 64/*108*/),
            &READ_QUAD_ADDR(5, INFINITE, 64/*108*/),
#endif
            0
        },

        {
            &WRITE_STD(0, 256, 64/*108*/),
            &WRITE_DUAL(0, 256, 64/*108*/),
            &WRITE_DUAL_ADDR(0, 256, 64/*108*/),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &WRITE_QUAD(0, 256, 64/*108*/),
			/* &WRITE_QUAD_ADDR(0, 256, 64), */
#endif
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 64/*108*/),
            0
        },
        &spi_driver_general,
    },

    {
		"N25Q064A",   {0x20, 0xbb, 0x17}, 3, (_64K * 128), _64K, 3,
		{
			&READ_STD(0, INFINITE, 54),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 80),
			&READ_DUAL_ADDR(2, INFINITE, 84),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&READ_QUAD(1, INFINITE, 80),
#endif
			0
		},
		{
			&WRITE_STD(0, 256, 80),
			&WRITE_DUAL(0, 256, 75),
			&WRITE_DUAL_ADDR(0, 256, 75),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&WRITE_QUAD(0, 256, 80),
#endif
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 80),
			0
		},
		&spi_driver_general,
	},
	{
		"MT(N)25Q128(AB)A11",   {0x20, 0xbb, 0x18}, 3,
		(_64K * 256), _64K, 3,
		{
			&READ_STD(0, INFINITE, 54),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 80),
			&READ_DUAL_ADDR(2, INFINITE, 84),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&READ_QUAD(1, INFINITE, 80),
			&READ_QUAD_ADDR(5, INFINITE, 84),
#endif
			0
		},
		{
			&WRITE_STD(0, 256, 80),
			&WRITE_DUAL(0, 256, 75),
			&WRITE_DUAL_ADDR(0, 256, 75),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&WRITE_QUAD(0, 256, 80),
#endif
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 80),
			0
		},
		&spi_driver_general,
	},
	{
		"N25QL064A",   {0x20, 0xba, 0x17}, 3, (_64K * 128), _64K, 3,
		{
			&READ_STD(0, INFINITE, 54),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 80),
			&READ_DUAL_ADDR(2, INFINITE, 84),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&READ_QUAD(1, INFINITE, 80),
			&READ_QUAD_ADDR(5, INFINITE, 84),
#endif
			0
		},
		{
			&WRITE_STD(0, 256, 80),
			&WRITE_DUAL(0, 256, 75),
			&WRITE_DUAL_ADDR(0, 256, 75),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&WRITE_QUAD(0, 256, 80),
#endif
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 50),
			0
		},
		&spi_driver_general,
	},
	{
		"N25QL128A",   {0x20, 0xba, 0x18}, 3, (_64K * 256), _64K, 3,
        {
            &READ_STD(0, INFINITE, 54),
            &READ_FAST(1, INFINITE, 108),
			&READ_DUAL(1, INFINITE, 84),
			&READ_DUAL_ADDR(2, INFINITE, 84),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&READ_QUAD(1, INFINITE, 84),
			&READ_QUAD_ADDR(5, INFINITE, 84),
#endif
            0
        },

        {
            &WRITE_STD(0, 256, 108),
            &WRITE_DUAL(0, 256, 108),
            &WRITE_DUAL_ADDR(0, 256, 108),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &WRITE_QUAD(0, 256, 108),
#endif
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 108),
            0
        },
		&spi_driver_general,
    },

	/* Micron MT25QL256A 3.3V */
        {
		"MT25QL256A",   {0x20, 0xba, 0x19}, 3, (_64K * 512), _64K, 4,
        {
            &READ_STD(0, INFINITE, 54),
			&READ_FAST(1, INFINITE, 133),
			&READ_DUAL(1, INFINITE, 133),
			&READ_DUAL_ADDR(2, INFINITE, 133),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&READ_QUAD(1, INFINITE, 133),
			&READ_QUAD_ADDR(5, INFINITE, 125),
#endif
            0
        },

        {
			&WRITE_STD(0, 256, 133),
			&WRITE_DUAL(0, 256, 133),
			&WRITE_DUAL_ADDR(0, 256, 133),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&WRITE_QUAD(0, 256, 133),
			/* &WRITE_QUAD_ADDR(0, 256, 133), */
#endif
            0
        },

        {
			&ERASE_SECTOR_64K(0, _64K, 133),
            0
        },
		&spi_driver_general,
    },

    {
        "M25PX16",  {0x20, 0x71, 0x15}, 3, (_64K * 32),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 33),
            &READ_FAST(1, INFINITE, 75),
            &READ_DUAL(1, INFINITE, 75),
            0
        },

        {
            &WRITE_STD(0, 256, 75),
            &WRITE_DUAL(0, 256, 75),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 75),
            0
        },
        &spi_driver_general,
    },

    {
        "M25PX32", {0x20, 0x71, 0x16}, 3, (_64K * 64),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 33),
            &READ_FAST(1, INFINITE, 75),
            &READ_DUAL(1, INFINITE, 75),
            0
        },

        {
            &WRITE_STD(0, 256, 75),
            &WRITE_DUAL(0, 256, 75),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 75),
            0
        },
        &spi_driver_general,
    },

    {
        "m25px64",  {0x20, 0x71, 0x17}, 3, (_64K * 128), _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

	{
		"MT25QU256A",   {0x20, 0xbb, 0x19}, 3, (_64K * 512), _64K, 4,
		{
			&READ_STD(0, INFINITE, 54),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 80),
			&READ_DUAL_ADDR(2, INFINITE, 84),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&READ_QUAD(1, INFINITE, 80),
			&READ_QUAD_ADDR(5, INFINITE, 84),
#endif
			0
		},
		{
			&WRITE_STD(0, 256, 80),
			&WRITE_DUAL(0, 256, 75),
			&WRITE_DUAL_ADDR(0, 256, 75),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&WRITE_QUAD(0, 256, 80),
#endif
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 80),
			0
		},
		&spi_driver_general,
	},
    /* Winbond -- w25x "blocks" are 64K, "sectors" are 4KiB */
    {
        "w25x10",  {0xef, 0x30, 0x11}, 3, (_64K * 2),    _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "w25x20",  {0xef, 0x30, 0x12}, 3, (_64K * 4),    _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

            {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "w25x40",  {0xef, 0x30, 0x13}, 3, (_64K * 8),    _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "w25x80",  {0xef, 0x30, 0x14}, 3, (_64K * 16),   _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

            {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "w25x16",  {0xef, 0x30, 0x15}, 3, (_64K * 32),   _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },

        {
            &WRITE_STD(0, 256, 0),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "w25x32",  {0xef, 0x30, 0x16}, 3, (_64K * 64),   _64K, 3,
        {
            &READ_STD(0, INFINITE, 0), 0},
        {
            &WRITE_STD(0, 256, 0),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "w25x64",  {0xef, 0x30, 0x17}, 3, (_64K * 128),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 0),
            0
        },
        {
            &WRITE_STD(0, 256, 0),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 0),
            0
        },
        &spi_driver_general,
    },

    {
        "W25Q80BV",  {0xef, 0x40, 0x14}, 3, (_64K * 16),   _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, 80),
#endif
            0
        },

        {
            &WRITE_STD(0, 256, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &WRITE_QUAD(0, 256, 80),
#endif
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 80),
            0
        },
        &spi_driver_general,
    },

    {
		"W25Q16(B/C/J)V/S25FL016K",
        {0xef, 0x40, 0x15}, 3, (_64K * 32), _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, 80),
#endif
            0
        },

        {
            &WRITE_STD(0, 256, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &WRITE_QUAD(0, 256, 80),
#endif
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 80),
            0
        },
        &spi_driver_general,
    },
    /*
     The follow chips have the same chipid, but command have some difference
    {
        "W25Q16BV",  {0xef, 0x40, 0x15}, 3, (_64K * 32),   _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 80),
            &READ_QUAD(1, INFINITE, 80),
            0
        },
        {
            &WRITE_STD(0, 256, 80),
            &WRITE_QUAD(0, 256, 80),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 80),
            0
        }
    },

    {
        "W25Q16CV",  {0xef, 0x40, 0x15}, 3, (_64K * 32),   _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 80),
            &READ_QUAD(1, INFINITE, 80),
            0
        },
        {
            &WRITE_STD(0, 256, 80),
            &WRITE_QUAD(0, 256, 80),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 80),
            0
        }
    },

    */
    {
		"W25Q32(B/F)V",  {0xef, 0x40, 0x16}, 3, (_64K * 64),   _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, 80),
#endif
            0
        },

        {
            &WRITE_STD(0, 256, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &WRITE_QUAD(0, 256, 80),
#endif
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 80),
            0
        },
        &spi_driver_general,
    },

        {
        "W25Q64FV",  {0xef, 0x40, 0x17}, 3, _8M,   _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, 80),
#endif
            0
        },

        {
            &WRITE_STD(0, 256, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &WRITE_QUAD(0, 256, 80),
#endif
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 80),
            0
        },
        &spi_driver_general,
    },

	{
		"W25Q64FW",  {0xef, 0x60, 0x17}, 3, _8M,   _64K, 3,
		{
			&READ_STD(0, INFINITE, 50),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&READ_QUAD(1, INFINITE, 80),
			&READ_QUAD_ADDR(3, INFINITE, 80),
#endif
			0
		},
		{
			&WRITE_STD(0, 256, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&WRITE_QUAD(0, 256, 80),
#endif
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 80),
			0
		},
		&spi_driver_w25q256fv,
	},
	{
		"W25Q128FW",  {0xef, 0x60, 0x18}, 3, _16M,   _64K, 3,
		{
			&READ_STD(0, INFINITE, 50),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 80),
			&READ_DUAL_ADDR(1, INFINITE, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&READ_QUAD(1, INFINITE, 80),
			&READ_QUAD_ADDR(3, INFINITE, 80),
#endif
			0
		},
		{
			&WRITE_STD(0, 256, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&WRITE_QUAD(0, 256, 80),
#endif
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 80),
			0
		},
		&spi_driver_w25q256fv,
	},
    {
        "W25Q128(B/F)V", {0xEF, 0x40, 0x18}, 3, _16M, _64K, 3,
        {
            &READ_STD(0, INFINITE, 33),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 104),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, /*70*/80),
#endif
            0
        },

        {
            &WRITE_STD(0, 256, 104),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &WRITE_QUAD(0, 256, /*70*/80),
#endif
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
		&spi_driver_w25q256fv,
    },
	{
		"W25Q128JV", {0xEF, 0x70, 0x18}, 3, _16M, _64K, 3,
		{
			&READ_STD(0, INFINITE, 33),
			&READ_FAST(1, INFINITE, 104),
			&READ_DUAL(1, INFINITE, 104),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&READ_QUAD(1, INFINITE, /*70*/80),
#endif
#ifdef CONFIG_DTR_MODE_SUPPORT
			&READ_QUAD_DTR_WINBOND(8, INFINITE, 80),
#endif
			0
		},
		{
			&WRITE_STD(0, 256, 104),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&WRITE_QUAD(0, 256, /*70*/80),
#endif
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 104),
			0
		},
		&spi_driver_w25q256fv,
	},

    {
        "W25Q256FV", {0xEF, 0x40, 0x19}, 3, _32M, _64K, 4,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, 80),
#endif
            0
        },

        {
            &WRITE_STD(0, 256, 104),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &WRITE_QUAD(0, 256, 80),
#endif
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_w25q256fv,
    },

    /* Eon -- fit clock frequency of RDSR instruction*/
    {
        "EN25F80", {0x1c, 0x31, 0x14}, 3, (_64K * 16),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 66),
            &READ_FAST(1, INFINITE, 66/*100*/),
            0
        },

        {
            &WRITE_STD(0, 256, 66/*100*/),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 66/*100*/),
            0
        },
        &spi_driver_general,
    },

    {
        "EN25F16", {0x1c, 0x31, 0x15}, 3, _2M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 66),
            &READ_FAST(1, INFINITE, 66/*100*/),
            0
        },

        {
            &WRITE_STD(0, 256, 66/*100*/),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 66/*100*/),
            0
        },
        &spi_driver_general,
    },

    {
        "EN25Q32B", {0x1c, 0x30, 0x16}, 3, (_64K * 64),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 80/*104*/),
            &READ_DUAL(1, INFINITE, 80),
            &READ_DUAL_ADDR(1, INFINITE, 80),
            /*&READ_QUAD(3, INFINITE, 80), */
            0
        },

        {
            &WRITE_STD(0, 256, 80/*104*/),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 80/*104*/),
            0
        },
        &spi_driver_general,
    },

    {
        "EN25Q64", {0x1c, 0x30, 0x17}, 3, (_64K * 128),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 100),
            &READ_DUAL(1, INFINITE, 80),
            &READ_DUAL_ADDR(1, INFINITE, 80),
            0
        },

        {
            &WRITE_STD(0, 256, 80),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_no_qe,
    },

    {
        "EN25Q128", {0x1c, 0x30, 0x18}, 3, (_64K * 256),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 80),
            &READ_DUAL_ADDR(1, INFINITE, 80),
            0
        },

        {
            &WRITE_STD(0, 256, 104),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_no_qe,
    },

    /* ESMT */
    {
        "F25L64QA", {0x8C, 0x41, 0x17}, 3, (_64K * 128),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 66),
            &READ_FAST(1, INFINITE, /*66*/100),
            &READ_DUAL(1, INFINITE, /*66*/80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, 80),
#endif
            0
        },
        {
            &WRITE_STD(0, 256, /*66*/100),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &WRITE_QUAD(0, 256, 80),
#endif
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, /*66*/100),
            0
        },
        &spi_driver_f25l64q,
	},

	/* GD GD25LQ128 1.8V*/
	{
		"GD25LQ128", {0xC8, 0x60, 0x18}, 3, _16M,  _64K, 3,
		{
			&READ_STD(0, INFINITE, 80),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 80),
			&READ_DUAL_ADDR(1, INFINITE, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&READ_QUAD(1, INFINITE, 80),
			&READ_QUAD_ADDR(3, INFINITE, 80),
#endif
			0
		},
		{
			&WRITE_STD(0, 256, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&WRITE_QUAD(0, 256, 80),
#endif
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 80),
			0
		},
		&spi_driver_gd25qxxx,
    },

    {
        "GD25Q128", {0xC8, 0x40, 0x18}, 3, _16M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 66),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, 80),
#endif
            0
        },
        {
            &WRITE_STD(0, 256, 100),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &WRITE_QUAD(0, 256, 80),
#endif
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 100),
            0
        },
        &spi_driver_gd25qxxx,
    },

    {
        "GD25Q64", {0xC8, 0x40, 0x17}, 3, _8M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 66),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, 80),
#endif
            0
        },
        {
            &WRITE_STD(0, 256, 100),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &WRITE_QUAD(0, 256, 80),
#endif
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 100),
            0
        },
        &spi_driver_gd25qxxx,
    },
	{
		"GD25Q16C", {0xC8, 0x40, 0x15}, 3, _2M,  _64K, 3,
		{
			&READ_STD(0, INFINITE, 80),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 80),
			&READ_DUAL_ADDR(1, INFINITE, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&READ_QUAD(1, INFINITE, 80),
			&READ_QUAD_ADDR(3, INFINITE, 80),
#endif
			0
		},
		{
			&WRITE_STD(0, 256, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&WRITE_QUAD(0, 256, 80),
#endif
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 80),
			0
		},
		&spi_driver_gd25qxxx,
	},
	{
		"GD25LQ64C", {0xC8, 0x60, 0x17}, 3, _8M,  _64K, 3,
		{
			&READ_STD(0, INFINITE, 80),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 80),
			&READ_DUAL_ADDR(1, INFINITE, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&READ_QUAD(1, INFINITE, 80),
			&READ_QUAD_ADDR(3, INFINITE, 80),
#endif
			0
		},
		{
			&WRITE_STD(0, 256, 80),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&WRITE_QUAD(0, 256, 80),
#endif
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 80),
			0
		},
		&spi_driver_gd25qxxx,
	},
    {
        "GD25Q32", {0xC8, 0x40, 0x16}, 3, _4M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 66),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, 80),
#endif
            0
        },
        {
            &WRITE_STD(0, 256, 100),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &WRITE_QUAD(0, 256, 80),
#endif
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 100),
            0
        },
		&spi_driver_gd25qxxx,
	},
	{
		"PN25F16S", {0xe0, 0x40, 0x15}, 3, _2M,  _64K, 3,
		{
			&READ_STD(0, INFINITE, 55),
			&READ_FAST(1, INFINITE, 108),
			&READ_DUAL(1, INFINITE, 108),
			&READ_DUAL_ADDR(1, INFINITE, 108),
			0
		},
		{
			&WRITE_STD(0, 256, 108),
			0
		},
		{
			&ERASE_SECTOR_64K(0, _64K, 108),
			0
		},
        &spi_driver_general,
    },
    {
		"PN25F32S", {0xe0, 0x40, 0x16}, 3, _4M,  _64K, 3,
        {
			&READ_STD(0, INFINITE, 55),
			&READ_FAST(1, INFINITE, 108),
			&READ_DUAL(1, INFINITE, 108),
			&READ_DUAL_ADDR(1, INFINITE, 108),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
			&READ_QUAD(1, INFINITE, 108),
			&READ_QUAD_ADDR(3, INFINITE, 108),
#endif
            0
        },

        {
			&WRITE_STD(0, 256, 108),
            0
        },
        {
			&ERASE_SECTOR_64K(0, _64K, 108),
            0
        },
        &spi_driver_general,
    },

    {0, {0}, 0, 0, 0, 0, {0}, {0}, {0}, NULL},
};

/*---------------------------------------------------------------------------*/
/* spi_nor_serach_id */
/*---------------------------------------------------------------------------*/
struct spi_nor_info *spi_nor_serach_id(char *id)
{
    struct spi_nor_info *info = spi_nor_info_table;

    for (; info->name; info++) {
        if (memcmp(info->id, id, info->id_len))
            continue;
        else
            return info;
    }
    return NULL;
}

/*---------------------------------------------------------------------------*/
/* spi_nor_search_rw */
/*---------------------------------------------------------------------------*/
void spi_nor_search_rw(struct spi_nor_info *info,
    struct spi_op *spiop_rw, uint32_t iftype, uint32_t max_dummy, int rw_type)
{
    int ix = 0;
    struct spi_op **spiop, **fitspiop;

    for (fitspiop = spiop = (rw_type ? info->write : info->read);
        (*spiop) && ix < MAX_SPI_OP; spiop++, ix++) {
        if (((*spiop)->iftype & iftype)
            && ((*spiop)->dummy <= max_dummy)
            && (*fitspiop)->iftype < (*spiop)->iftype)
            fitspiop = spiop;
    }
    memcpy(spiop_rw, (*fitspiop), sizeof(struct spi_op));
}

/*---------------------------------------------------------------------------*/
/* spi_nor_get_erase */
/*---------------------------------------------------------------------------*/
void spi_nor_get_erase(struct spi_nor_info *info,
        struct spi_op *spiop_erase)
{
    int ix;

    spiop_erase->size = 0;
    for (ix = 0; ix < MAX_SPI_OP; ix++) {
        if (info->erase[ix] == NULL)
            break;
        if (info->erasesize == info->erase[ix]->size) {
            memcpy(&spiop_erase[ix], info->erase[ix],
                    sizeof(struct spi_op));
            break;
        }
    }
}

/*---------------------------------------------------------------------------*/
/* spinor_get_dev_info_by_id - [DEFAULT] Get spinor device information by ID */
/*---------------------------------------------------------------------------*/
struct spinor_dev_info *spinor_get_dev_info_by_id(struct spinor_info *spinor)
{
    char *id = spinor->dev.id;
    struct spi_nor_info *find;
    struct spinor_dev_info *dev = &spinor->dev;

    MTD_PR(INIT_DBG, "\t *-Start find spinor flash\n");

    INFO_MSG("Spi Nor ID:");
    for(int i=0; i<SPI_NOR_MAX_ID_LEN; i++)
        INFO_MSG("0x%02X ",id[i]);
    INFO_MSG("\n");

    find = spi_nor_serach_id(id);

    if(find) {
        dev->name = find->name;
        dev->id_len = find->id_len;
        dev->chipsize = find->chipsize;
        dev->blocksize = find->erasesize;
        dev->priv = find;

        spinor->ids_probe(spinor);

        INFO_MSG("Spi Nor Flash Info:\n");
        INFO_MSG("Name:\"%s\" ", spinor->dev.name);
        INFO_MSG("Size:%sB ", ulltostr(spinor->dev.chipsize));
        INFO_MSG("Block:%sB\n", ulltostr(spinor->dev.blocksize));

        return dev;
    }
    ERR_MSG("Not found spinor flash!\n");
    return NULL;
}
