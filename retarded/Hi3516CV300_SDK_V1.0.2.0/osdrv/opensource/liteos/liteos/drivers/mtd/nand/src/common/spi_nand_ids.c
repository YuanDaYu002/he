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

#include "stdio.h"
#include "string.h"

#include "mtd_common.h"
#include "nand_common.h"
#include "host_common.h"

#include "spi_nand_ids.h"
/*****************************************************************************/
SET_READ_STD(1, INFINITE, 24);

SET_READ_FAST(1, INFINITE, 60);
SET_READ_FAST(1, INFINITE, 75);
SET_READ_FAST(1, INFINITE, 80);
SET_READ_FAST(1, INFINITE, 104);
SET_READ_FAST(1, INFINITE, 108);
SET_READ_FAST(1, INFINITE, 120);

SET_READ_DUAL(1, INFINITE, 60);
SET_READ_DUAL(1, INFINITE, 75);
SET_READ_DUAL(1, INFINITE, 80);
SET_READ_DUAL(1, INFINITE, 104);
SET_READ_DUAL(1, INFINITE, 108);
SET_READ_DUAL(1, INFINITE, 120);

SET_READ_DUAL_ADDR(1, INFINITE, 60);
SET_READ_DUAL_ADDR(1, INFINITE, 75);
SET_READ_DUAL_ADDR(1, INFINITE, 80);
SET_READ_DUAL_ADDR(1, INFINITE, 104);
SET_READ_DUAL_ADDR(1, INFINITE, 108);
SET_READ_DUAL_ADDR(1, INFINITE, 120);

SET_READ_QUAD(1, INFINITE, 60);
SET_READ_QUAD(1, INFINITE, 75);
SET_READ_QUAD(1, INFINITE, 80);
SET_READ_QUAD(1, INFINITE, 104);
SET_READ_QUAD(1, INFINITE, 108);
SET_READ_QUAD(1, INFINITE, 120);

SET_READ_QUAD_ADDR(1, INFINITE, 60);
SET_READ_QUAD_ADDR(1, INFINITE, 75);
SET_READ_QUAD_ADDR(2, INFINITE, 75);
SET_READ_QUAD_ADDR(1, INFINITE, 80);
SET_READ_QUAD_ADDR(2, INFINITE, 80);
SET_READ_QUAD_ADDR(2, INFINITE, 104);
SET_READ_QUAD_ADDR(1, INFINITE, 108);
SET_READ_QUAD_ADDR(1, INFINITE, 120);

/*****************************************************************************/
SET_WRITE_STD(0, 256, 24);
SET_WRITE_STD(0, 256, 75);
SET_WRITE_STD(0, 256, 80);
SET_WRITE_STD(0, 256, 104);

SET_WRITE_QUAD(0, 256, 75);
SET_WRITE_QUAD(0, 256, 80);
SET_WRITE_QUAD(0, 256, 104);
SET_WRITE_QUAD(0, 256, 108);
SET_WRITE_QUAD(0, 256, 120);

/*****************************************************************************/
SET_ERASE_SECTOR_128K(0, _128K, 24);
SET_ERASE_SECTOR_128K(0, _128K, 75);
SET_ERASE_SECTOR_128K(0, _128K, 80);
SET_ERASE_SECTOR_128K(0, _128K, 104);

SET_ERASE_SECTOR_256K(0, _256K, 24);
SET_ERASE_SECTOR_256K(0, _256K, 75);
SET_ERASE_SECTOR_256K(0, _256K, 80);
SET_ERASE_SECTOR_256K(0, _256K, 104);

/*****************************************************************************/
extern int spinand_general_wait_ready(struct spi *spi);
extern int spinand_general_write_enable(struct spi *spi);
extern int spinand_general_qe_enable(struct spi *spi);
extern int spinand_qe_not_enable(struct spi *spi);

static struct spi_drv spi_driver_general = {
    .wait_ready = spinand_general_wait_ready,
    .write_enable = spinand_general_write_enable,
    .qe_enable = spinand_general_qe_enable,
};

static struct spi_drv spi_driver_no_qe = {
    .wait_ready = spinand_general_wait_ready,
    .write_enable = spinand_general_write_enable,
    .qe_enable = spinand_qe_not_enable,
};

/*****************************************************************************/
#define SPI_NAND_ID_TAB_VER		"2.4"

/******* SPI Nand ID Table ***************************************************
* Version	Manufacturer	Chip Name	Size		Operation
* 1.0		ESMT		F50L512M41A	64MB		Add 5 chip
*		GD		5F1GQ4UAYIG	128MB
*		GD		5F2GQ4UAYIG	256MB
*		GD		5F4GQ4UAYIG	512MB
*		GD		5F4GQ4UBYIG	512MB
* 1.1		ESMT		F50L1G41A	128MB		Add 2 chip
*		Winbond		W25N01GV	128MB
* 1.2		GD		5F1GQ4UBYIG	128MB		Add 2 chip
*		GD		5F2GQ4UBYIG	256MB
* 1.3		ATO		ATO25D1GA	128MB		Add 1 chip
* 1.4		MXIC		MX35LF1GE4AB	128MB		Add 2 chip
*		MXIC		MX35LF2GE4AB	256MB		(SOP-16Pin)
* 1.5		Paragon		PN26G01A	128MB		Add 1 chip
* 1.6		All-flash	AFS1GQ4UAC	128MB		Add 1 chip
* 1.7		TOSHIBA		TC58CVG0S3H	128MB		Add 2 chip
*		TOSHIBA		TC58CVG2S0H	512MB
* 1.8		ALL-flash	AFS2GQ4UAD	256MB		Add 2 chip
*		Paragon		PN26G02A	256MB
* 1.9		TOSHIBA		TC58CVG1S3H	256MB		Add 1 chip
* 2.0		HeYangTek	HYF1GQ4UAACAE	128MB		Add 3 chip
*		HeYangTek	HYF2GQ4UAACAE	256MB
*		HeYangTek	HYF4GQ4UAACBE	512MB
* 2.1		Micron		MT29F1G01ABA	128MB		Add 5 chip
		Paragon	1.8V	PN26Q01AWSIUG	128MB
		TOSHIBA 1.8V	TC58CYG0S3H	128MB
		TOSHIBA 1.8V	TC58CYG1S3H	256MB
		TOSHIBA 1.8V	TC58CYG2S0H	512MB
* 2.2		Micron		MT29F2G01ABA	256MB		Add 1 chip
* 2.3		MXIC		MX35LF2G14AC	256MB		Add 1 chip
* 2.4	    GD 1.8V		5F4GQ4RAYIG		512MB		Add 1 chip
******************************************************************************/
struct spi_nand_info spi_nand_flash_table[] = {
	/* Micron MT29F1G01ABA 1GBit */
	{
		.name      = "MT29F1G01ABA",
		.id        = {0x2C, 0x14},
		.id_len    = 2,
		.chipsize  = _128M,
		.blocksize = _128K,
		.pagesize  = _2K,
		.oobsize   = 128,
		.badblock_pos = BBP_FIRST_PAGE,
		.read      = {
			&READ_STD(1, INFINITE, 24),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 80),
			&READ_DUAL_ADDR(1, INFINITE, 80),
			&READ_QUAD(1, INFINITE, 80),
			&READ_QUAD_ADDR(2, INFINITE, 80),
			0
		},
		.write     = {
			&WRITE_STD(0, 256, 80),
			&WRITE_QUAD(0, 256, 80),
			0
		},
		.erase     = {
			&ERASE_SECTOR_128K(0, _128K, 80),
			0
		},
		.driver    = &spi_driver_no_qe,
	},

	/* Micron MT29F2G01ABA 2GBit */
	{
		.name      = "MT29F2G01ABA",
		.id        = {0x2C, 0x24},
		.id_len    = 2,
		.chipsize  = _256M,
		.blocksize = _128K,
		.pagesize  = _2K,
		.oobsize   = 128,
		.badblock_pos = BBP_FIRST_PAGE,
		.read      = {
			&READ_STD(1, INFINITE, 24),
			&READ_FAST(1, INFINITE, 108),
			&READ_DUAL(1, INFINITE, 108),
			&READ_DUAL_ADDR(1, INFINITE, 108),
			&READ_QUAD(1, INFINITE, 108),
			&READ_QUAD_ADDR(2, INFINITE, 104),
			0
		},
		.write     = {
			&WRITE_STD(0, 256, 80),
			&WRITE_QUAD(0, 256, 108),
			0
		},
		.erase     = {
			&ERASE_SECTOR_128K(0, _128K, 80),
			0
		},
		.driver    = &spi_driver_no_qe,
	},
    /* ESMT F50L512M41A 512Mbit */
    {
        .name      = "F50L512M41A",
        .id        = {0xC8, 0x20},
        .id_len    = 2,
        .chipsize  = _64M,
        .blocksize = _128K,
        .pagesize  = _2K,
        .oobsize   = 64,
        .badblock_pos = BBP_FIRST_PAGE,
        .read      = {
            &READ_STD(1, INFINITE, 24),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 104),
            &READ_QUAD(1, INFINITE, 104),
            0
        },
        .write     = {
            &WRITE_STD(0, 256, 24),
            &WRITE_QUAD(0, 256, 104),
            0
        },
        .erase     = {
            &ERASE_SECTOR_128K(0, _128K, 24),
            0
        },
        .driver    = &spi_driver_no_qe,
    },

    /* ESMT F50L1G41A 1Gbit */
    {
        .name      = "F50L1G41A",
        .id        = {0xC8, 0x21},
        .id_len    = 2,
        .chipsize  = _128M,
        .blocksize = _128K,
        .pagesize  = _2K,
        .oobsize   = 64,
        .badblock_pos = BBP_FIRST_PAGE,
        .read      = {
            &READ_STD(1, INFINITE, 24),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 104),
            &READ_QUAD(1, INFINITE, 104),
            0
        },
        .write     = {
            &WRITE_STD(0, 256, 24),
            &WRITE_QUAD(0, 256, 104),
            0
        },
        .erase     = {
            &ERASE_SECTOR_128K(0, _128K, 24),
            0
        },
        .driver    = &spi_driver_no_qe,
    },

    /* GD 5F1GQ4UAYIG 1Gbit */
    {
        .name      = "5F1GQ4UAYIG",
        .id        = {0xc8, 0xf1},
        .id_len    = 2,
        .chipsize  = _128M,
        .blocksize = _128K,
        .pagesize  = _2K,
        .oobsize   = 64,
        .badblock_pos = BBP_FIRST_PAGE,
        .read      = {
            &READ_STD(1, INFINITE, 24),
            &READ_FAST(1, INFINITE, 120),
            &READ_DUAL(1, INFINITE, 120),
            &READ_DUAL_ADDR(1, INFINITE, 120),
            &READ_QUAD(1, INFINITE, 120),
            &READ_QUAD_ADDR(1, INFINITE, 120),
            0
        },
        .write     = {
            &WRITE_STD(0, 256, 24),
            &WRITE_QUAD(0, 256, 120),
            0
        },
        .erase     = {
            &ERASE_SECTOR_128K(0, _128K, 24),
            0
        },
        .driver    = &spi_driver_general,
    },

    /* GD 5F1GQ4UBYIG 1Gbit */
    {
        .name      = "5F1GQ4UBYIG",
        .id        = {0xc8, 0xd1},
        .id_len    = 2,
        .chipsize  = _128M,
        .blocksize = _128K,
        .pagesize  = _2K,
        .oobsize   = 128,
        .badblock_pos = BBP_FIRST_PAGE,
        .read      = {
            &READ_STD(1, INFINITE, 24),
            &READ_FAST(1, INFINITE, 120),
            &READ_DUAL(1, INFINITE, 120),
            &READ_DUAL_ADDR(1, INFINITE, 120),
            &READ_QUAD(1, INFINITE, 120),
            &READ_QUAD_ADDR(1, INFINITE, 120),
            0
        },
        .write     = {
            &WRITE_STD(0, 256, 24),
            &WRITE_QUAD(0, 256, 120),
            0
        },
        .erase     = {
            &ERASE_SECTOR_128K(0, _128K, 24),
            0
        },
        .driver    = &spi_driver_general,
    },

    /* GD 5F2GQ4UAYIG 2Gbit */
    {
        .name      = "5F2GQ4UAYIG",
        .id        = {0xc8, 0xf2},
        .id_len    = 2,
        .chipsize  = _256M,
        .blocksize = _128K,
        .pagesize  = _2K,
        .oobsize   = 64,
        .badblock_pos = BBP_FIRST_PAGE,
        .read      = {
            &READ_STD(1, INFINITE, 24),
            &READ_FAST(1, INFINITE, 120),
            &READ_DUAL(1, INFINITE, 120),
            &READ_DUAL_ADDR(1, INFINITE, 120),
            &READ_QUAD(1, INFINITE, 120),
            &READ_QUAD_ADDR(1, INFINITE, 120),
            0
        },
        .write     = {
            &WRITE_STD(0, 256, 24),
            &WRITE_QUAD(0, 256, 120),
            0
        },
        .erase     = {
            &ERASE_SECTOR_128K(0, _128K, 24),
            0
        },
        .driver    = &spi_driver_general,
    },

    /* GD 5F2GQ4UBYIG 2Gbit */
    {
        .name      = "5F2GQ4UBYIG",
        .id        = {0xc8, 0xd2},
        .id_len    = 2,
        .chipsize  = _256M,
        .blocksize = _128K,
        .pagesize  = _2K,
        .oobsize   = 128,
        .badblock_pos = BBP_FIRST_PAGE,
        .read      = {
            &READ_STD(1, INFINITE, 24),
            &READ_FAST(1, INFINITE, 120),
            &READ_DUAL(1, INFINITE, 120),
            &READ_DUAL_ADDR(1, INFINITE, 120),
            &READ_QUAD(1, INFINITE, 120),
            &READ_QUAD_ADDR(1, INFINITE, 120),
            0
        },
        .write     = {
            &WRITE_STD(0, 256, 24),
            &WRITE_QUAD(0, 256, 120),
            0
        },
        .erase     = {
            &ERASE_SECTOR_128K(0, _128K, 24),
            0
        },
        .driver    = &spi_driver_general,
    },

    /* GD 5F4GQ4UAYIG 4Gbit */
    {
        .name      = "5F4GQ4UAYIG",
        .id        = {0xc8, 0xf4},
        .id_len    = 2,
        .chipsize  = _512M,
        .blocksize = _128K,
        .pagesize  = _2K,
        .oobsize   = 64,
        .badblock_pos = BBP_FIRST_PAGE,
        .read      = {
            &READ_STD(1, INFINITE, 24),
            &READ_FAST(1, INFINITE, 120),
            &READ_DUAL(1, INFINITE, 120),
            &READ_DUAL_ADDR(1, INFINITE, 120),
            &READ_QUAD(1, INFINITE, 120),
            &READ_QUAD_ADDR(1, INFINITE, 120),
            0
        },
        .write     = {
            &WRITE_STD(0, 256, 24),
            &WRITE_QUAD(0, 256, 120),
            0
        },
        .erase     = {
            &ERASE_SECTOR_128K(0, _128K, 24),
            0
        },
        .driver    = &spi_driver_general,
    },

    /* GD 5F4GQ4UBYIG 4Gbit */
    {
        .name      = "5F4GQ4UBYIG",
        .id        = {0xc8, 0xd4},
        .id_len    = 2,
        .chipsize  = _512M,
        .blocksize = _256K,
        .pagesize  = _4K,
        .oobsize   = 256,
        .badblock_pos = BBP_FIRST_PAGE,
        .read      = {
            &READ_STD(1, INFINITE, 24),
            &READ_FAST(1, INFINITE, 120),
            &READ_DUAL(1, INFINITE, 120),
            &READ_DUAL_ADDR(1, INFINITE, 120),
            &READ_QUAD(1, INFINITE, 120),
            &READ_QUAD_ADDR(1, INFINITE, 120),
            0
        },
        .write     = {
            &WRITE_STD(0, 256, 24),
            &WRITE_QUAD(0, 256, 120),
            0
        },
        .erase     = {
            &ERASE_SECTOR_256K(0, _256K, 24),
            0
        },
        .driver    = &spi_driver_general,
    },

	{
		.name      = "5F4GQ4RAYIG",
		.id        = {0xc8, 0xe4},
		.id_len    = 2,
		.chipsize  = _512M,
		.blocksize = _256K,
		.pagesize  = _4K,
		.oobsize   = 256,
		.badblock_pos = BBP_FIRST_PAGE,
		.read      = {
			&READ_STD(1, INFINITE, 24),
			&READ_FAST(1, INFINITE, 75),
			&READ_DUAL(1, INFINITE, 75),
			&READ_DUAL_ADDR(1, INFINITE, 75),
			&READ_QUAD(1, INFINITE, 75),
			&READ_QUAD_ADDR(1, INFINITE, 75),
			0
		},
		.write     = {
			&WRITE_STD(0, 256, 24),
			&WRITE_QUAD(0, 256, 75),
			0
		},
		.erase     = {
			&ERASE_SECTOR_256K(0, _256K, 24),
			0
		},
		.driver    = &spi_driver_general,
	},
    /* Winbond W25N01GV 1Gbit */
    {
        .name      = "W25N01GV",
        .id        = {0xef, 0xaa, 0x21},
        .id_len    = 3,
        .chipsize  = _128M,
        .blocksize = _128K,
        .pagesize  = _2K,
        .oobsize   = 64,
        .badblock_pos = BBP_FIRST_PAGE,
        .read      = {
            &READ_STD(1, INFINITE, 24),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 104),
            &READ_DUAL_ADDR(1, INFINITE, 104),
            &READ_QUAD(1, INFINITE, 104),
            &READ_QUAD_ADDR(2, INFINITE, 104),
            0
        },
        .write     = {
            &WRITE_STD(0, 256, 24),
            &WRITE_QUAD(0, 256, 104),
            0
        },
        .erase     = {
            &ERASE_SECTOR_128K(0, _128K, 24),
            0
        },
		.driver    = &spi_driver_no_qe,
    },

	/* Winbond W25N01GW 1Gbit 1.8V */
	{
		.name      = "W25N01GW",
		.id        = {0xef, 0xba, 0x21},
		.id_len    = 3,
		.chipsize  = _128M,
		.blocksize = _128K,
		.pagesize  = _2K,
		.oobsize   = 64,
		.badblock_pos = BBP_FIRST_PAGE,
		.read      = {
			&READ_STD(1, INFINITE, 24),
			&READ_FAST(1, INFINITE, 75),
			&READ_DUAL(1, INFINITE, 75),
			&READ_DUAL_ADDR(1, INFINITE, 75),
			&READ_QUAD(1, INFINITE, 75),
			&READ_QUAD_ADDR(2, INFINITE, 75),
			0
		},
		.write     = {
			&WRITE_STD(0, 256, 24),
			&WRITE_QUAD(0, 256, 75),
			0
		},
		.erase     = {
			&ERASE_SECTOR_128K(0, _128K, 24),
			0
		},
		.driver    = &spi_driver_no_qe,
	},
    /* ATO ATO25D1GA 1Gbit */
    {
        .name      = "ATO25D1GA",
        .id        = {0x9b, 0x12},
        .id_len    = 2,
        .chipsize  = _128M,
        .blocksize = _128K,
        .pagesize  = _2K,
        .oobsize   = 64,
        .badblock_pos = BBP_FIRST_PAGE,
        .read      = {
            &READ_STD(1, INFINITE, 24),
            &READ_FAST(1, INFINITE, 104),
            &READ_QUAD(1, INFINITE, 104),
            0
        },
        .write     = {
            &WRITE_STD(0, 256, 24),
            &WRITE_QUAD(0, 256, 104),
            0
        },
        .erase     = {
            &ERASE_SECTOR_128K(0, _128K, 24),
            0
        },
        .driver    = &spi_driver_general,
    },

    /* MXIC MX35LF1GE4AB 1Gbit */
    {
        .name      = "MX35LF1GE4AB",
        .id        = {0xc2, 0x12},
        .id_len    = 2,
        .chipsize  = _128M,
        .blocksize = _128K,
        .pagesize  = _2K,
        .oobsize   = 64,
        .badblock_pos = BBP_FIRST_PAGE,
        .read      = {
            &READ_STD(1, INFINITE, 24),
            &READ_FAST(1, INFINITE, 104),
            &READ_QUAD(1, INFINITE, 104),
            0
        },
        .write     = {
            &WRITE_STD(0, 256, 24),
            &WRITE_QUAD(0, 256, 104),
            0
        },
        .erase     = {
            &ERASE_SECTOR_128K(0, _128K, 24),
            0
        },
        .driver    = &spi_driver_general,
    },

    /* MXIC MX35LF2GE4AB 2Gbit */
    {
        .name      = "MX35LF2GE4AB",
        .id        = {0xc2, 0x22},
        .id_len    = 2,
        .chipsize  = _256M,
        .blocksize = _128K,
        .pagesize  = _2K,
        .oobsize   = 64,
        .badblock_pos = BBP_FIRST_PAGE,
        .read      = {
            &READ_STD(1, INFINITE, 24),
            &READ_FAST(1, INFINITE, 104),
            &READ_QUAD(1, INFINITE, 104),
            0
        },
        .write     = {
            &WRITE_STD(0, 256, 24),
            &WRITE_QUAD(0, 256, 104),
            0
        },
        .erase     = {
            &ERASE_SECTOR_128K(0, _128K, 24),
            0
        },
        .driver    = &spi_driver_general,
    },

	{
		.name      = "MX35LF2G14AC",
		.id        = {0xc2, 0x20},
		.id_len    = 2,
		.chipsize  = _256M,
		.blocksize = _128K,
		.pagesize  = _2K,
		.oobsize   = 64,
		.badblock_pos = BBP_FIRST_PAGE,
		.read      = {
			&READ_STD(1, INFINITE, 24),
			&READ_FAST(1, INFINITE, 104),
			&READ_QUAD(1, INFINITE, 104),
			0
		},
		.write     = {
			&WRITE_STD(0, 256, 24),
			&WRITE_QUAD(0, 256, 104),
			0
		},
		.erase     = {
			&ERASE_SECTOR_128K(0, _128K, 24),
			0
		},
		.driver    = &spi_driver_general,
	},
	/* Paragon PN26Q01AWSIUG 1Gbit 1.8V */
	{
		.name      = "PN26G01AW",
		.id        = {0xa1, 0xc1},
		.id_len    = 2,
		.chipsize  = _128M,
		.blocksize = _128K,
		.pagesize  = _2K,
		.oobsize   = 128,
		.badblock_pos = BBP_FIRST_PAGE,
		.read      = {
			&READ_STD(1, INFINITE, 24),
			&READ_FAST(1, INFINITE, 75),
			&READ_DUAL(1, INFINITE, 75),
			&READ_DUAL_ADDR(1, INFINITE, 75),
			&READ_QUAD(1, INFINITE, 75),
			&READ_QUAD_ADDR(1, INFINITE, 75),
			0
		},
		.write     = {
			&WRITE_STD(0, 256, 24),
			&WRITE_QUAD(0, 256, 75),
			0
		},
		.erase     = {
			&ERASE_SECTOR_128K(0, _128K, 75),
			0
		},
		.driver    = &spi_driver_general,
	},

	/* Paragon PN26G01A 1Gbit */
	{
        .name      = "PN26G01A",
        .id        = {0xa1, 0xe1},
        .id_len    = 2,
        .chipsize  = _128M,
        .blocksize = _128K,
        .pagesize  = _2K,
        .oobsize   = 128,
        .badblock_pos = BBP_FIRST_PAGE,
        .read      = {
            &READ_STD(1, INFINITE, 24),
            &READ_FAST(1, INFINITE, 108),
            &READ_DUAL(1, INFINITE, 108),
            &READ_DUAL_ADDR(1, INFINITE, 108),
            &READ_QUAD(1, INFINITE, 108),
            &READ_QUAD_ADDR(1, INFINITE, 108),
            0
        },
        .write     = {
            &WRITE_STD(0, 256, 24),
            &WRITE_QUAD(0, 256, 108),
            0
        },
        .erase     = {
            &ERASE_SECTOR_128K(0, _128K, 24),
            0
        },
        .driver    = &spi_driver_general,
    },
	/* Paragon PN26G02A 2Gbit */
    {
        .name      = "PN26G02A",
        .id        = {0xa1, 0xe2},
        .id_len    = 2,
        .chipsize  = _256M,
        .blocksize = _128K,
        .pagesize  = _2K,
        .oobsize   = 128,
        .badblock_pos = BBP_FIRST_PAGE,
        .read      = {
            &READ_STD(1, INFINITE, 24),
            &READ_FAST(1, INFINITE, 108),
            &READ_DUAL(1, INFINITE, 108),
            &READ_DUAL_ADDR(1, INFINITE, 108),
            &READ_QUAD(1, INFINITE, 108),
            &READ_QUAD_ADDR(1, INFINITE, 108),
            0
        },
        .write     = {
            &WRITE_STD(0, 256, 24),
            &WRITE_QUAD(0, 256, 108),
            0
        },
        .erase     = {
            &ERASE_SECTOR_128K(0, _128K, 24),
            0
        },
        .driver    = &spi_driver_general,
    },
	/* All-flash AFS1GQ4UAC 1Gbit */
    {
        .name      = "AFS1GQ4UAC",
        .id        = {0xc1, 0x51},
        .id_len    = 2,
        .chipsize  = _128M,
        .blocksize = _128K,
        .pagesize  = _2K,
        .oobsize   = 128,
        .badblock_pos = BBP_FIRST_PAGE,
        .read      = {
            &READ_STD(1, INFINITE, 24),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 80),
			&READ_DUAL_ADDR(1, INFINITE, 80),
			&READ_QUAD(1, INFINITE, 80),
			&READ_QUAD_ADDR(1, INFINITE, 80),
            0
        },
        .write     = {
            &WRITE_STD(0, 256, 24),
			&WRITE_QUAD(0, 256, 80),
            0
        },
        .erase     = {
            &ERASE_SECTOR_128K(0, _128K, 24),
            0
        },
        .driver    = &spi_driver_general,
    },
	/* All-flash AFS2GQ4UAD 2Gbit */
    {
        .name      = "AFS2GQ4UAD",
        .id        = {0xc1, 0x52},
        .id_len    = 2,
        .chipsize  = _256M,
        .blocksize = _128K,
        .pagesize  = _2K,
        .oobsize   = 128,
        .badblock_pos = BBP_FIRST_PAGE,
        .read      = {
            &READ_STD(1, INFINITE, 24),
			&READ_FAST(1, INFINITE, 80),
			&READ_DUAL(1, INFINITE, 80),
			&READ_DUAL_ADDR(1, INFINITE, 80),
			&READ_QUAD(1, INFINITE, 80),
			&READ_QUAD_ADDR(1, INFINITE, 80),
			0
        },
        .write     = {
            &WRITE_STD(0, 256, 24),
			&WRITE_QUAD(0, 256, 80),
            0
        },
        .erase     = {
            &ERASE_SECTOR_128K(0, _128K, 24),
            0
        },
        .driver    = &spi_driver_general,
    },
	/* TOSHIBA TC58CVG0S3H 1Gbit */
    {
        .name      = "TC58CVG0S3H",
        .id        = {0x98, 0xc2},
        .id_len    = 2,
        .chipsize  = _128M,
        .blocksize = _128K,
        .pagesize  = _2K,
        .oobsize   = 64,
        .badblock_pos = BBP_FIRST_PAGE,
        .read      = {
            &READ_STD(1, INFINITE, 24),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 104),
            &READ_QUAD(1, INFINITE, 104),
            0
        },
        .write     = {
            &WRITE_STD(0, 256, 104),
            0
        },
        .erase     = {
            &ERASE_SECTOR_128K(0, _128K, 104),
            0
        },
        .driver    = &spi_driver_no_qe,
    },

	/* TOSHIBA TC58CYG0S3H 1.8V 1Gbit */
	{
		.name      = "TC58CYG0S3H",
		.id        = {0x98, 0xb2},
		.id_len    = 2,
		.chipsize  = _128M,
		.blocksize = _128K,
		.pagesize  = _2K,
		.oobsize   = 128,
		.badblock_pos = BBP_FIRST_PAGE,
		.read      = {
			&READ_STD(1, INFINITE, 24),
			&READ_FAST(1, INFINITE, 75),
			&READ_DUAL(1, INFINITE, 75),
			&READ_QUAD(1, INFINITE, 75),
			0
		},
		.write     = {
			&WRITE_STD(0, 256, 75),
			0
		},
		.erase     = {
			&ERASE_SECTOR_128K(0, _128K, 75),
			0
		},
		.driver    = &spi_driver_no_qe,
	},
	/* TOSHIBA TC58CVG1S3H 2Gbit */
	{
		.name      = "TC58CVG1S3H",
		.id        = {0x98, 0xcb},
		.id_len    = 2,
		.chipsize  = _256M,
		.blocksize = _128K,
		.pagesize  = _2K,
		.oobsize   = 64,
		.badblock_pos = BBP_FIRST_PAGE,
		.read      = {
			&READ_STD(1, INFINITE, 24),
			&READ_FAST(1, INFINITE, 104),
			&READ_DUAL(1, INFINITE, 104),
			&READ_QUAD(1, INFINITE, 104),
			0
		},
		.write     = {
			&WRITE_STD(0, 256, 104),
			0
		},
		.erase     = {
			&ERASE_SECTOR_128K(0, _128K, 104),
			0
		},
		.driver    = &spi_driver_no_qe,
	},

	/* TOSHIBA TC58CYG1S3H 1.8V 2Gbit */
	{
		.name      = "TC58CYG1S3H",
		.id        = {0x98, 0xbb},
		.id_len    = 2,
		.chipsize  = _256M,
		.blocksize = _128K,
		.pagesize  = _2K,
		.oobsize   = 128,
		.badblock_pos = BBP_FIRST_PAGE,
		.read      = {
			&READ_STD(1, INFINITE, 24),
			&READ_FAST(1, INFINITE, 75),
			&READ_DUAL(1, INFINITE, 75),
			&READ_QUAD(1, INFINITE, 75),
			0
		},
		.write     = {
			&WRITE_STD(0, 256, 75),
			0
		},
		.erase     = {
			&ERASE_SECTOR_128K(0, _128K, 75),
			0
		},
		.driver    = &spi_driver_no_qe,
	},

	/* TOSHIBA TC58CVG2S0H 4Gbit */
    {
        .name      = "TC58CVG2S0H",
        .id        = {0x98, 0xcd},
        .id_len    = 2,
        .chipsize  = _512M,
        .blocksize = _256K,
        .pagesize  = _4K,
        .oobsize   = 128,
        .badblock_pos = BBP_FIRST_PAGE,
        .read      = {
            &READ_STD(1, INFINITE, 24),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 104),
            &READ_QUAD(1, INFINITE, 104),
            0
        },
        .write     = {
            &WRITE_STD(0, 256, 104),
            0
        },
        .erase     = {
            &ERASE_SECTOR_256K(0, _256K, 104),
            0
        },
        .driver    = &spi_driver_no_qe,
    },
	/* TOSHIBA TC58CYG2S0H 1.8V 4Gbit */
	{
		.name      = "TC58CYG2S0H",
		.id        = {0x98, 0xbd},
		.id_len    = 2,
		.chipsize  = _512M,
		.blocksize = _256K,
		.pagesize  = _4K,
		.oobsize   = 256,
		.badblock_pos = BBP_FIRST_PAGE,
		.read      = {
			&READ_STD(1, INFINITE, 24),
			&READ_FAST(1, INFINITE, 75),
			&READ_DUAL(1, INFINITE, 75),
			&READ_QUAD(1, INFINITE, 75),
			0
		},
		.write     = {
			&WRITE_STD(0, 256, 75),
			0
		},
		.erase     = {
			&ERASE_SECTOR_256K(0, _256K, 75),
			0
		},
		.driver    = &spi_driver_no_qe,
	},

	/* HeYangTek HYF1GQ4UAACAE 1Gbit */
	{
		.name      = "HYF1GQ4UAACAE",
		.id        = {0xc9, 0x51},
		.id_len    = 2,
		.chipsize  = _128M,
		.blocksize = _128K,
		.pagesize  = _2K,
		.oobsize   = 128,
		.badblock_pos = BBP_FIRST_PAGE,
		.read      = {
			&READ_STD(1, INFINITE, 24),
			&READ_FAST(1, INFINITE, 60),
			&READ_DUAL(1, INFINITE, 60),
			&READ_DUAL_ADDR(1, INFINITE, 60),
			&READ_QUAD(1, INFINITE, 60),
			&READ_QUAD_ADDR(1, INFINITE, 60),
			0
		},
		.write     = {
			&WRITE_STD(0, 256, 80),
			&WRITE_QUAD(0, 256, 80),
			0
		},
		.erase     = {
			&ERASE_SECTOR_128K(0, _128K, 80),
			0
		},
		.driver    = &spi_driver_general,
	},

	/* HeYangTek HYF2GQ4UAACAE 2Gbit */
	{
		.name      = "HYF2GQ4UAACAE",
		.id        = {0xc9, 0x52},
		.id_len    = 2,
		.chipsize  = _256M,
		.blocksize = _128K,
		.pagesize  = _2K,
		.oobsize   = 128,
		.badblock_pos = BBP_FIRST_PAGE,
		.read      = {
			&READ_STD(1, INFINITE, 24),
			&READ_FAST(1, INFINITE, 60),
			&READ_DUAL(1, INFINITE, 60),
			&READ_DUAL_ADDR(1, INFINITE, 60),
			&READ_QUAD(1, INFINITE, 60),
			&READ_QUAD_ADDR(1, INFINITE, 60),
			0
		},
		.write     = {
			&WRITE_STD(0, 256, 80),
			&WRITE_QUAD(0, 256, 80),
			0
		},
		.erase     = {
			&ERASE_SECTOR_128K(0, _128K, 80),
			0
		},
		.driver    = &spi_driver_general,
	},

	/* HeYangTek HYF4GQ4UAACBE 4Gbit */
	{
		.name      = "HYF4GQ4UAACBE",
		.id        = {0xc9, 0xd4},
		.id_len    = 2,
		.chipsize  = _512M,
		.blocksize = _256K,
		.pagesize  = _4K,
		.oobsize   = 256,
		.badblock_pos = BBP_FIRST_PAGE,
		.read      = {
			&READ_STD(1, INFINITE, 24),
			&READ_FAST(1, INFINITE, 60),
			&READ_DUAL(1, INFINITE, 60),
			&READ_DUAL_ADDR(1, INFINITE, 60),
			&READ_QUAD(1, INFINITE, 60),
			&READ_QUAD_ADDR(1, INFINITE, 60),
			0
		},
		.write     = {
			&WRITE_STD(0, 256, 80),
			&WRITE_QUAD(0, 256, 80),
			0
		},
		.erase     = {
			&ERASE_SECTOR_256K(0, _256K, 80),
			0
		},
		.driver    = &spi_driver_general,
	},
    {    .id_len    = 0,    },
};

/*****************************************************************************/
void spi_nand_search_rw(struct spi_nand_info *spiinfo,
    struct spi_op *spiop_rw, u_int iftype, u_int max_dummy, int rw_type)
{
    int ix = 0;
    struct spi_op **spiop, **fitspiop;

    for (fitspiop = spiop = (rw_type ? spiinfo->write : spiinfo->read);
        (*spiop) && ix < MAX_SPI_OP; spiop++, ix++) {
        if (((*spiop)->iftype & iftype)
            && ((*spiop)->dummy <= max_dummy)
            && (*fitspiop)->iftype < (*spiop)->iftype)
            fitspiop = spiop;
        }

    memcpy(spiop_rw, (*fitspiop), sizeof(struct spi_op));
}

/*****************************************************************************/
void spi_nand_get_erase(struct spi_nand_info *spiinfo,
        struct spi_op *spiop_erase)
{
    int ix;

    spiop_erase->size = 0;
    for (ix = 0; ix < MAX_SPI_OP; ix++) {
        if (spiinfo->erase[ix] == NULL)
            break;
        if (spiinfo->blocksize == spiinfo->erase[ix]->size) {
            memcpy(&spiop_erase[ix], spiinfo->erase[ix],
                    sizeof(struct spi_op));
            break;
        }
    }
}

/*---------------------------------------------------------------------------*/
/* nand_get_dev_info_by_id - [DEFAULT] Get Nand device information by ID */
/*---------------------------------------------------------------------------*/
struct nand_dev_info *nand_get_dev_info_by_id(struct nand_info *nand)
{
    char *id = nand->dev.id;
    struct spi_nand_info *cur;
    struct nand_dev_info *find = &nand->dev;

    MTD_PR(INIT_DBG, "\t *-Start find SPI Nand flash\n");

    INFO_MSG("Nand ID:");
    for(int i=0; i<MAX_SPI_NAND_ID_LEN; i++)
        INFO_MSG("0x%02X ",id[i]);
    INFO_MSG("\n");

    for (cur = spi_nand_flash_table; cur->id_len; cur++) {
        if (cur->id[0] && memcmp(id,cur->id,cur->id_len))
            continue;

        if ((cur->id_len == 2) && (id[1] != cur->id[1]))
            continue;
        MTD_PR(INIT_DBG, "\t *-Found Nand: %s\n", cur->name);

        find->name = cur->name;
        find->id_len = cur->id_len;
        find->chipsize = cur->chipsize;
        find->pagesize = cur->pagesize;
        //if (find->pagesize) {
            find->oobsize = cur->oobsize;
            find->blocksize = cur->blocksize;
        //}

        find->page_shift = ffs(find->pagesize) - 1;
        find->pagemask = find->pagesize -1;
        find->block_shift = ffs(find->blocksize) - 1;
        find->blockmask = find->blocksize -1;
        if (find->chipsize & 0xffffffff)
            find->chip_shift = ffs((unsigned)find->chipsize) - 1;
        else
            find->chip_shift = ffs((unsigned)(find->chipsize >> 32))
                        + 31;

        find->priv = cur;
        nand->ids_probe(nand);

        MTD_PR(INIT_DBG, "\t *-End of found SPI nand flash\n");

        INFO_MSG("Nand:\"%s\"\n", nand->dev.name);
        INFO_MSG("Size:%sB ", ulltostr(nand->dev.chipsize));
        INFO_MSG("Block:%sB ", ulltostr(nand->dev.blocksize));
        INFO_MSG("Page:%sB ", ulltostr(nand->dev.pagesize));
        INFO_MSG("Oob:%sB ", ulltostr(nand->dev.oobsize));

        return find;
    }

    ERR_MSG("Not found SPI nand flash!!!\n");

    return NULL;
}

