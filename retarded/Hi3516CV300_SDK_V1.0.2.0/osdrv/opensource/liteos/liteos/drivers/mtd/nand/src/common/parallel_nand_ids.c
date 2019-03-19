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

#include "stdlib.h"
#include "string.h"

#include "parallel_nand_ids.h"

/*---------------------------------------------------------------------------*/
/*****************************************************************************/
/*
 * samsung:  27nm need randomizer, 21nm need read retry;
 * micron:   25nm need read retry, datasheet will explain read retry.
 * toshaba   32nm need randomizer, 24nm need read retry.
 * hynix:    2xnm need read retry.
 *
 *		The special nand flash ID table version 1.38
 *
 * manufactory  |  type  |       name 	     |   ecc_type  | version_tag
 * Micron		|  MLC	 |  MT29F64G08CBABA  |   40bit/1k  |  1.36
 * Micron		|  MLC	 |  MT29F32G08CBADA  |   40bit/1k  |
 * Micron		|  SLC	 |  MT29F8G08ABxBA   |   4bit/512  |
 * Micron		|  MLC	 |  MT29F16G08CBABx  |   12bit/512 |
 * Micron		|  MLC	 |  MT29F16G08CBACA  |   24bit/1k  |
 * Micron		|  MLC	 |  MT29F32G08CBACA  |   24bit/1k  |
 * Micron		|  MLC	 |  MT29F64G08CxxAA  |   24bit/1k  |
 * Micron		|  MLC	 |  MT29F256G08CJAAA |   24bit/1k  |   2CE
 * Micron		|  MLC	 |  MT29F256G08CMCBB |   24bit/1k  |
 * Micron		|  SLC	 |  MT29F8G08ABACA   |   8bit/512  |
 * Micron		|  SLC	 |  MT29F4G08ABAEA   |   8bit/512  |
 * Micron		|  SLC	 |  MT29F2G08ABAFA   |   8bit/512  |
 * Micron		|  SLC	 |  MT29F16G08ABACA  |   8bit/512  |
 * Toshiba		|  MLC   |  TC58NVG4D2FTA00  |   24bit/1k  |
 * Toshiba		|  MLC   |  TH58NVG6D2FTA20  |   24bit/1k  |   2CE
 * Toshiba		|  MLC   |  TC58NVG5D2HTA00  |   40bit/1k  |
 * Toshiba		|  MLC   |  TC58NVG6D2GTA00  |   40bit/1k  |
 * Toshiba		|  MLC   |  TC58NVG6DCJTA00  |			   |
 * Toshiba		|  MLC   |  TC58TEG5DCJTA00  |			   |
 * Toshiba		|  SLC   |  TC58NVG0S3HTA00  |   8bit/512  |
 * Toshiba		|  SLC   |  TC58NVG1S3HTA00  |   8bit/512  |
 * Toshiba		|  SLC   |  TC58NVG1S3ETA00  |   4bit/512  |
 * Toshiba		|  SLC   |  TC58NVG3S0FTA00  |   4bit/512  |
 * Toshiba		|  SLC   |  TC58NVG2S0FTA00  |   4bit/512  |
 * Toshiba		|  SLC   |  TH58NVG2S3HTA00  |   4bit/512  |
 * Toshiba		|  TLC   |  TC58NVG5T2JTA00  |   60bit/1k  |
 * Toshiba		|  TLC   |  TC58TEG5DCKTAx0  |   60bit/1k  |
 * Toshiba		|  MLC   |  Tx58TEGxDDKTAx0  |			   |
 * Samsung		|  MLC   |  K9LB(HC/PD/MD)G08U0(1)D  |   8bit/512B  |
 * Samsung		|  MLC   |  K9GAG08U0E	     |   24bit/1KB |
 * Samsung		|  MLC   |  K9LBG08U0E	     |   24bit/1KB |
 * Samsung		|  MLC   |  K9G8G08U0C	     |   24bit/1KB |
 * Samsung		|  MLC   |  K9GAG08U0F	     |   24bit/1KB |
 * Samsung		|  MLC   |  K9LBG08U0M	     |             |
 * Samsung		|  MLC   |  K9GBG08U0A	     |   24bit/1KB |
 * Samsung		|  MLC   |  K9GBG08U0B	     |   40bit/1KB |
 * Hynix		|  MLC   |  H27UAG8T2A	     |			   |
 * Hynix		|  MLC   |  H27UAG8T2B	     |			   |
 * Hynix		|  MLC   |  H27UBG8T2A	     |			   |
 * Hynix		|  MLC   |  H27UBG8T2BTR	 |	 24bit/1KB |
 * Hynix		|  MLC   |  H27UCG8T2A		 |	 40bit/1KB |
 * Hynix		|  MLC   |  H27UBG8T2C		 |	 40bit/1KB |
 * MISC			|  MLC   |  P1UAGA30AT-GCA	 |	 8bit/512  |
 * MISC			|  MLC   |  PSU8GA30AT-GIA/ASU8GA30IT-G30CA	 |	 4bit/512  |
 * MISC			|  SLC   |  PSU2GA30AT   	 |	 1bit/512  |   1.36
 * Toshiba		|  SLC   |  TC58NVG2S0HTA00  |	 24bit/1K  |   1.37
 * Toshiba		|  SLC   |  TC58NVG3S0HTA00  |   24bit/1K  |   1.37
 * Micron		|  SLC	 |  MT29F2G08ABAEA   |   4bit/512 |
 * Spansion		|  SLC	 | S34ML02G200TFI000	 | 24bit/1K |
 * Spansion		|  SLC	 | S34ML04G200TFI000	 | 24bit/1K |  1.38
 *
 */
static struct nand_flash_info nand_flash_info_t[] = {
	{		/* SLC S34ML02G200TFI000 */
		.name      = "S34ML02G200TFI000",
		.id        = {0x01, 0xDA, 0x90, 0x95, 0x46, 0x00, 0x00, 0x00},
		.id_len    = 5,
		.chipsize  = _256M,
		.pagesize  = _2K,
		.blocksize = _128K,
		.oobsize   = 128,
		.badblock_pos    = BBP_FIRST_PAGE,
		.flags = 0,
	},
	{		/* SLC S34ML04G200TFI000 */
		.name      = "S34ML04G200TFI000",
		.id        = {0x01, 0xDC, 0x90, 0x95, 0x56, 0x00, 0x00, 0x00},
		.id_len    = 5,
		.chipsize  = _512M,
		.pagesize  = _2K,
		.blocksize = _128K,
		.oobsize   = 128,
		.badblock_pos    = BBP_FIRST_PAGE,
		.flags = 0,
	},
    {        /* MLC 40bit/1k */
        .name      = "MT29F64G08CBABA",
        .id        = {0x2C, 0x64, 0x44, 0x4B, 0xA9, 0x00, 0x00, 0x00},
        .id_len    = 8,
        .chipsize  = _8G,
        .pagesize  = _8K,
        .blocksize = _2M,
        .oobsize   = 744,
        .badblock_pos = BBP_FIRST_PAGE,
        .flags = NAND_RANDOMIZER | NAND_SYNCHRONOUS | NAND_ASYNCHRONOUS,
    },
    {        /* MLC 40bit/1k */
        .name      = "MT29F32G08CBADA",
        .id        = {0x2C, 0x44, 0x44, 0x4B, 0xA9, 0x00, 0x00, 0x00},
        .id_len    = 8,
        .chipsize  = _4G,
        .pagesize  = _8K,
        .blocksize = _2M,
        .oobsize   = 744,
        .badblock_pos = BBP_FIRST_PAGE,
        .flags = NAND_RANDOMIZER,
    },
    {        /* SLC 4bit/512 */
        .name      = "MT29F8G08ABxBA",
        .id        = {0x2C, 0x38, 0x00, 0x26, 0x85, 0x00, 0x00, 0x00},
        .id_len    = 8,
        .chipsize  = _1G,
        .pagesize  = _4K,
        .blocksize = _512K,
        .oobsize   = 224,
        .badblock_pos = BBP_FIRST_PAGE,
    },
    {        /* MLC 12bit/512 */
        .name      = "MT29F16G08CBABx",
        .id        = {0x2C, 0x48, 0x04, 0x46, 0x85, 0x00, 0x00, 0x00},
        .id_len    = 8,
        .chipsize  = _2G,
        .pagesize  = _4K,
        .blocksize = _1M,
        .oobsize   = 224,
        .badblock_pos = BBP_FIRST_PAGE,
    },
    {        /* MLC 24bit/1k */
        .name      = "MT29F16G08CBACA",
        .id        = {0x2C, 0x48, 0x04, 0x4A, 0xA5, 0x00, 0x00, 0x00},
        .id_len    = 8,
        .chipsize  = _2G,
        .pagesize  = _4K,
        .blocksize = _1M,
        .oobsize   = 224,
        .badblock_pos = BBP_FIRST_PAGE,
    },
    {        /* MLC 24bit/1k */
        .name      = "MT29F32G08CBACA",
        .id        = {0x2C, 0x68, 0x04, 0x4A, 0xA9, 0x00, 0x00, 0x00},
        .id_len    = 8,
        .chipsize  = _4G,
        .pagesize  = _4K,
        .blocksize = _1M,
        .oobsize   = 224,
        .badblock_pos = BBP_FIRST_PAGE,
    },
    {        /* MLC 24bit/1k */
        .name      = "MT29F64G08CxxAA",
        .id        = {0x2C, 0x88, 0x04, 0x4B, 0xA9, 0x00, 0x00, 0x00},
        .id_len    = 8,
        .chipsize  = _8G,
        .pagesize  = _8K,
        .blocksize = _2M,
        .oobsize   = 448,
        .badblock_pos = BBP_FIRST_PAGE,
    },
    {        /* MLC 24bit/1k 2CE */
        .name      = "MT29F256G08CJAAA",
        .id        = {0x2C, 0xA8, 0x05, 0xCB, 0xA9, 0x00, 0x00, 0x00},
        .id_len    = 8,
        .chipsize  = _16G,
        .pagesize  = _8K,
        .blocksize = _2M,
        .oobsize   = 448,
        .badblock_pos = BBP_FIRST_PAGE,
    },
    {        /* MLC 40bit/1k */
        .name      = "MT29F256G08CMCBB",
        .id        = {0x2C, 0x64, 0x44, 0x4B, 0xA9, 0x00, 0x00, 0x00},
        .id_len    = 8,
        .chipsize  = _8G,
        .pagesize  = _8K,
        .blocksize = _2M,
        .oobsize   = 744,
        .badblock_pos = BBP_FIRST_PAGE,
    },
    {        /* SLC 8bit/512 */
        .name      = "MT29F8G08ABACA",
        .id        = {0x2C, 0xD3, 0x90, 0xA6, 0x64, 0x00, 0x00, 0x00},
        .id_len    = 5,
        .chipsize  = _1G,
        .pagesize  = _4K,
        .blocksize = _256K,
        .oobsize   = 224,
        .badblock_pos = BBP_FIRST_PAGE,
    },
    {        /* SLC 8bit/512 */
        .name      = "MT29F4G08ABAEA",
        .id        = {0x2C, 0xDC, 0x90, 0xA6, 0x54, 0x00, 0x00, 0x00},
        .id_len    = 5,
        .chipsize  = _512M,
        .pagesize  = _4K,
        .blocksize = _256K,
        .oobsize   = 224,
        .badblock_pos = BBP_FIRST_PAGE,
    },
    {        /* SLC 8bit/512 */
        .name      = "MT29F2G08ABAFA",
        .id        = {0x2C, 0xDA, 0x90, 0x95, 0x04, 0x00, 0x00, 0x00},
        .id_len    = 5,
        .chipsize  = _256M,
        .pagesize  = _2K,
        .blocksize = _128K,
        .oobsize   = 224,
        .badblock_pos = BBP_FIRST_PAGE,
    },
	{		/* SLC MT29F2G08ABAEA */
		.name      = "MT29F2G08ABAEA",
		.id        = {0x2C, 0xDA, 0x90, 0x95, 0x06, 0x00, 0x00, 0x00},
		.id_len    = 5,
		.chipsize  = _256M,
		.pagesize  = _2K,
		.blocksize = _128K,
		.oobsize   = 64,
		.badblock_pos    = BBP_FIRST_PAGE,
		.flags = 0,
	},
    {        /* SLC 8bit/512 */
        .name      = "MT29F16G08ABACA",
        .id        = {0x2C, 0x48, 0x00, 0x26, 0xA9, 0x00, 0x00, 0x00},
        .id_len    = 5,
        .chipsize  = _2G,
        .pagesize  = _4K,
        .blocksize = _512K,
        .oobsize   = 224,
        .badblock_pos = BBP_FIRST_PAGE,
    },

    /****************************** Toshaba *******************************/

    {       /* MLC 24bit/1k 32nm */
        .name      = "TC58NVG4D2FTA00",
        .id        = {0x98, 0xD5, 0x94, 0x32, 0x76, 0x55, 0x00, 0x00},
        .id_len    = 6,
        .chipsize  = _2G,
        .pagesize  = _8K,
        .blocksize = _1M,
        .oobsize   = 448,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
    },
    {       /* MLC 24bit/1k 32nm 2CE*/
        .name      = "TH58NVG6D2FTA20",
        .id        = {0x98, 0xD7, 0x94, 0x32, 0x76, 0x55, 0x00, 0x00},
        .id_len    = 6,
        .chipsize  = _4G,
        .pagesize  = _8K,
        .blocksize = _1M,
        .oobsize   = 448,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
    },
    {       /* MLC 40bit/1k 24nm */
        .name      = "TC58NVG5D2HTA00 24nm",
        .id        = {0x98, 0xD7, 0x94, 0x32, 0x76, 0x56, 0x08, 0x00},
        .id_len    = 6,
        .chipsize  = _4G,
        .pagesize  = _8K,
        .blocksize = _1M,
        .oobsize   = 640,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
        .flags = NAND_RANDOMIZER,
    },
    {       /* MLC 40bit/1k */
        .name      = "TC58NVG6D2GTA00",
        .id        = {0x98, 0xDE, 0x94, 0x82, 0x76, 0x00, 0x00, 0x00},
        .id_len    = 5,
        .chipsize  = _8G,
        .pagesize  = _8K,
        .blocksize = _2M,
        .oobsize   = 640,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
    },
    {       /* MLC 19nm */
        .name      = "TC58NVG6DCJTA00 19nm",
        .id        = {0x98, 0xDE, 0x84, 0x93, 0x72, 0x57, 0x08, 0x04},
        .id_len    = 8,
        .chipsize  = _8G,
        .pagesize  = _16K,
        .blocksize = _4M,
        .oobsize   = 1280,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
        .flags = NAND_RANDOMIZER,
    },
    {       /* MLC 19nm */
        .name      = "TC58TEG5DCJTA00 19nm",
        .id        = {0x98, 0xD7, 0x84, 0x93, 0x72, 0x57, 0x08, 0x04},
        .id_len    = 6,
        .chipsize  = _4G,
        .pagesize  = _16K,
        .blocksize = _4M,
        .oobsize   = 1280,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
        .flags = NAND_RANDOMIZER | NAND_SYNCHRONOUS | NAND_ASYNCHRONOUS,
    },
    {       /* SLC 8bit/512 */
        .name      = "TC58NVG0S3HTA00",
        .id        = {0x98, 0xF1, 0x80, 0x15, 0x72, 0x00, 0x00, 0x00},
        .id_len    = 5,
        .chipsize  = _128M,
        .pagesize  = _2K,
        .blocksize = _128K,
        .oobsize   = 128,
        /*
         * Datasheet: read one column of any page in each block. If the
         * data of the column is 00 (Hex),define the block as a bad
         * block.
         */
        .badblock_pos = BBP_FIRST_PAGE,
    },
    {       /* SLC 8bit/512 */
        .name      = "TC58NVG1S3HTA00",
        .id        = {0x98, 0xDA, 0x90, 0x15, 0x76, 0x16, 0x08, 0x00},
        .id_len    = 7,
        .chipsize  = _256M,
        .pagesize  = _2K,
        .blocksize = _128K,
        .oobsize   = 128,
        .badblock_pos = BBP_FIRST_PAGE,
    },
    {       /* SLC 4bit/512 */
        .name      = "TC58NVG1S3ETA00",
        .id        = {0x98, 0xDA, 0x90, 0x15, 0x76, 0x14, 0x03, 0x00},
        .id_len    = 7,
        .chipsize  = _256M,
        .pagesize  = _2K,
        .blocksize = _128K,
        .oobsize   = 64,
        .badblock_pos = BBP_FIRST_PAGE,
    },
    {       /* SLC 4bit/512 */
        .name      = "TC58NVG3S0FTA00",
        .id        = {0x98, 0xD3, 0x90, 0x26, 0x76, 0x15, 0x02, 0x08},
        .id_len    = 8,
        .chipsize  = _1G,
        .pagesize  = _4K,
        .blocksize = _256K,
        .oobsize   = 232,
        .badblock_pos = BBP_FIRST_PAGE,
    },
	{       /* SLC 24bit/1k */
		.name      = "TC58NVG3S0HTA00",
		.id        = {0x98, 0xD3, 0x91, 0x26, 0x76, 0x16, 0x08, 0x00},
		.id_len    = 8,
		.chipsize  = _1G,
		.pagesize  = _4K,
		.blocksize = _256K,
		.oobsize   = 256,
		.badblock_pos    = BBP_FIRST_PAGE,
		.flags = 0,
	},
{
        .name      = "TC58NVG2S0HTA00",
        .id        = {0x98, 0xDC, 0x90, 0x26, 0x76, 0x16, 0x08, 0x00},
        .id_len    = 8,
        .chipsize  = _512M,
        .pagesize  = _4K,
        .blocksize = _256K,
        .oobsize   = 256,
        .badblock_pos = BBP_FIRST_PAGE,
    },
    {       /* SLC 4bit/512 */
        .name      = "TC58NVG2S0FTA00",
        .id        = {0x98, 0xDC, 0x90, 0x26, 0x76, 0x15, 0x01, 0x08},
        .id_len    = 8,
        .chipsize  = _512M,
        .pagesize  = _4K,
        .blocksize = _256K,
        .oobsize   = 224,
        .badblock_pos = BBP_FIRST_PAGE,
    },
	{       /* SLC 4bit/512 */
		.name      = "TH58NVG2S3HTA00",
		.id        = {0x98, 0xDC, 0x91, 0x15, 0x76},
		.id_len    = 5,
		.chipsize  = _512M,
		.pagesize  = _2K,
		.blocksize = _128K,
		.oobsize   = 128,
		.badblock_pos    = BBP_FIRST_PAGE,
		.flags = 0,
	},
    {       /* TLC 60bit/1k 19nm */
        .name      = "TC58NVG5T2JTA00 19nm TLC",
        .id        = {0x98, 0xD7, 0x98, 0x92, 0x72, 0x57, 0x08, 0x10},
        .id_len    = 6,
        .chipsize  = _4G,
        .pagesize  = _8K,
        .blocksize = _4M,
        .oobsize   = 1024,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
        .flags = NAND_RANDOMIZER,
    },
	{	/* TLC 60bit/1k 19nm */
		.name	   = "TC58TEG5DCKTAx0 19nm MLC",
		.id	   = {0x98, 0xD7, 0x84, 0x93, 0x72, 0x50, 0x08, 0x04},
		.id_len    = 6,
		.chipsize  = _4G,
		.pagesize  = _16K,
		.blocksize = _4M,
		.oobsize   = 1280,
		.badblock_pos	 = BBP_FIRST_PAGE | BBP_LAST_PAGE,
		.flags = NAND_RANDOMIZER,
	},
	{
		.name	   = "Tx58TEGxDDKTAx0 19nm MLC",
		.id	   = {0x98, 0xDE, 0x94, 0x93, 0x76, 0x50},
		.id_len    = 6,
		.chipsize  = _4G,
		.pagesize  = _16K,
		.blocksize = _4M,
		.oobsize   = 1280,
		.badblock_pos	 = BBP_FIRST_PAGE | BBP_LAST_PAGE,
		.flags = NAND_RANDOMIZER,
	},
    /******************************* Samsung ******************************/
    {       /* MLC 8bit/512B */
        .name     = "K9LB(HC/PD/MD)G08U0(1)D",
        .id       = {0xEC, 0xD7, 0xD5, 0x29, 0x38, 0x41, 0x00, 0x00},
        .id_len   = 6,
        .chipsize = _4G,
        .badblock_pos = BBP_LAST_PAGE,
    },
    {       /* MLC 24bit/1KB */
        .name      = "K9GAG08U0E",
        .id        = {0xEC, 0xD5, 0x84, 0x72, 0x50, 0x42, 0x00, 0x00},
        .id_len    = 6,
        .chipsize  = _2G,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
    },
    {       /* MLC 24bit/1KB */
        .name     = "K9LBG08U0E",
        .id       = {0xEC, 0xD7, 0xC5, 0x72, 0x54, 0x42, 0x00, 0x00},
        .id_len   = 6,
        .chipsize = _4G,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
    },
    {       /* MLC 24bit/1KB */
        .name     = "K9G8G08U0C",
        .id       = {0xEC, 0xD3, 0x84, 0x72, 0x50, 0x42, 0x00, 0x00},
        .id_len   = 6,
        .chipsize = _1G,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
    },
    {        /* MLC 24bit/1k */
        .name      = "K9GAG08U0F",
        .id        = {0xEC, 0xD5, 0x94, 0x76, 0x54, 0x43, 0x00, 0x00},
        .id_len    = 6,
        .chipsize  = _2G,
        .pagesize  = _8K,
        .blocksize = _1M,
        .oobsize   = 512,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
    },
    {        /* MLC */
        .name      = "K9LBG08U0M",
        .id        = {0xEC, 0xD7, 0x55, 0xB6, 0x78, 0x00, 0x00, 0x00},
        .id_len    = 5,
        .chipsize  = _4G,
        .pagesize  = _4K,
        .blocksize = _512K,
        .oobsize   = 128,
        .badblock_pos = BBP_LAST_PAGE,
    },
    {        /* MLC 24bit/1k */
        .name      = "K9GBG08U0A 20nm",
        .id        = {0xEC, 0xD7, 0x94, 0x7A, 0x54, 0x43, 0x00, 0x00},
        .id_len    = 6,
        .chipsize  = _4G,
        .pagesize  = _8K,
        .blocksize = _1M,
        .oobsize   = 640,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
        .flags = NAND_RANDOMIZER,
    },
    {        /* MLC 40bit/1k */
        .name      = "K9GBG08U0B",
        .id        = {0xEC, 0xD7, 0x94, 0x7E, 0x64, 0x44, 0x00, 0x00},
        .id_len    = 6,
        .chipsize  = _4G,
        .pagesize  = _8K,
        .blocksize = _1M,
        .oobsize   = 1024,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
        .flags = NAND_RANDOMIZER,
    },

    /*********************************** Hynix ****************************/
    {       /* MLC */
        .name     = "H27UAG8T2A",
        .id       = {0xAD, 0xD5, 0x94, 0x25, 0x44, 0x41,},
        .id_len   = 6,
        .chipsize = _2G,
        //.probe    = hynix_probe_v02,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
    },
    {       /* MLC */
        .name     = "H27UAG8T2B",
        .id       = {0xAD, 0xD5, 0x94, 0x9A, 0x74, 0x42,},
        .id_len   = 6,
        .chipsize = _2G,
        //.probe    = hynix_probe_v02,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
    },
    {       /* MLC */
        .name     = "H27UBG8T2A",
        .id       = {0xAD, 0xD7, 0x94, 0x9A, 0x74, 0x42,},
        .id_len   = 6,
        .chipsize = _4G,
        //.probe    = hynix_probe_v02,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
    },
    {       /* MLC 24bit/1K,26nm TODO: Need read retry,chip is EOS */
        .name      = "H27UBG8T2BTR 26nm",
        .id        = {0xAD, 0xD7, 0x94, 0xDA, 0x74, 0xC3,},
        .id_len    = 6,
        .chipsize  = _4G,
        .pagesize  = _8K,
        .blocksize = _2M,
        .oobsize   = 640,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
        .flags = NAND_RANDOMIZER,
    },
    {        /* MLC 40bit/1k */
        .name      = "H27UCG8T2A",
        .id        = {0xAD, 0xDE, 0x94, 0xDA, 0x74, 0xC4,},
        .id_len    = 6,
        .chipsize  = _8G,
        .pagesize  = _8K,
        .blocksize = _2M,
        .oobsize   = 640,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
        .flags = NAND_RANDOMIZER,
    },
    {        /* MLC 40bit/1k */
        .name      = "H27UBG8T2C",
        .id        = {0xAD, 0xD7, 0x94, 0x91, 0x60, 0x44,},
        .id_len    = 6,
        .chipsize  = _4G,
        .pagesize  = _8K,
        .blocksize = _2M,
        .oobsize   = 640,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
        .flags = NAND_RANDOMIZER,
    },

    /********************** MISC ******************************************/
    {        /* MLC 8bit/512 */
        .name      = "P1UAGA30AT-GCA",
        .id        = {0xC8, 0xD5, 0x14, 0x29, 0x34, 0x01,},
        .id_len    = 6,
        .chipsize  = _2G,
        .pagesize  = _4K,
        .blocksize = _512K,
        .oobsize   = 218,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
    },
    {       /* MLC 4bit/512 */
        /*
         * PowerFlash ASU8GA30IT-G30CA ID and MIRA PSU8GA30AT-GIA ID are
         * the same ID
         */
        .name      = "PSU8GA30AT-GIA/ASU8GA30IT-G30CA",
        .id        = {0xC8, 0xD3, 0x90, 0x19, 0x34, 0x01,},
        .id_len    = 6,
        .chipsize  = _1G,
        .pagesize  = _4K,
        .blocksize = _256K,
        .oobsize   = 218,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
    },
    {        /* SLC 1bit/512 */
        .name      = "PSU2GA30AT",
        .id        = {0x7F, 0x7F, 0x7F, 0x7F, 0xC8, 0xDA, 0x00, 0x15,},
        .id_len    = 8,
        .chipsize  = _256M,
        .pagesize  = _2K,
        .blocksize = _128K,
        .oobsize   = 64,
        .badblock_pos = BBP_FIRST_PAGE | BBP_LAST_PAGE,
    },
    {
        .name = "NAND 1MiB 5V 8-bit",
        .id = {0, 0x6e,},
        .id_len = 2,
        .pagesize = _256B,
        .chipsize = _1M,
        .blocksize = _4K,
    },
    {
        .name = "NAND 2MiB 5V 8-bit",
        .id = {0, 0x64,},
        .id_len = 2,
        .pagesize = _256B,
        .chipsize = _2M,
        .blocksize = _4K,
    },
    {
        .name = "NAND 4MiB 5V 8-bit",
        .id = {0, 0x6b,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _4M,
        .blocksize = _8K,
    },
    {
        .name = "NAND 1MiB 3,3V 8-bit",
        .id = {0, 0xe8,},
        .id_len = 2,
        .pagesize = _256B,
        .chipsize = _1M,
        .blocksize = _4K,
    },
    {
        .name = "NAND 1MiB 3,3V 8-bit",
        .id = {0, 0xec,},
        .id_len = 2,
        .pagesize = _256B,
        .chipsize = _1M,
        .blocksize = _4K,
    },
    {
        .name = "NAND 2MiB 3,3V 8-bit",
        .id = {0, 0xea,},
        .id_len = 2,
        .pagesize = _256B,
        .chipsize = _2M,
        .blocksize = _4K,
    },
    {
        .name = "NAND 4MiB 3,3V 8-bit",
        .id = {0, 0xd5,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _4M,
        .blocksize = _8K,
    },
    {
        .name = "NAND 4MiB 3,3V 8-bit",
        .id = {0, 0xe3,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _4M,
        .blocksize = _8K,
    },
    {
        .name = "NAND 4MiB 3,3V 8-bit",
        .id = {0, 0xe5,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _4M,
        .blocksize = _8K,
    },
    {
        .name = "NAND 8MiB 3,3V 8-bit",
        .id = {0, 0xd6,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _8M,
        .blocksize = _8K,
    },
    {
        .name = "NAND 8MiB 1,8V 8-bit",
        .id = {0, 0x39,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _8M,
        .blocksize = _8K,
    },
    {
        .name = "NAND 8MiB 3,3V 8-bit",
        .id = {0, 0xe6,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _8M,
        .blocksize = _8K,
    },
    {
        .name = "NAND 8MiB 1,8V 16-bit",
        .id = {0, 0x49,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _8M,
        .blocksize = _8K,
    },
    {
        .name = "NAND 8MiB 3,3V 16-bit",
        .id = {0, 0x59,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _8M,
        .blocksize = _8K,
    },
    {
        .name = "NAND 16MiB 1,8V 8-bit",
        .id = {0, 0x33,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _16M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 16MiB 3,3V 8-bit",
        .id = {0, 0x73,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _16M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 16MiB 1,8V 16-bit",
        .id = {0, 0x43,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _16M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 16MiB 3,3V 16-bit",
        .id = {0, 0x53,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _16M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 32MiB 1,8V 8-bit",
        .id = {0, 0x35,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _32M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 32MiB 3,3V 8-bit",
        .id = {0, 0x75,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _32M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 32MiB 1,8V 16-bit",
        .id = {0, 0x45,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _32M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 32MiB 3,3V 16-bit",
        .id = {0, 0x55,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _32M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 64MiB 1,8V 8-bit",
        .id = {0, 0x36,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _64M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 64MiB 3,3V 8-bit",
        .id = {0, 0x76,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _64M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 64MiB 1,8V 16-bit",
        .id = {0, 0x46,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _64M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 64MiB 3,3V 16-bit",
        .id = {0, 0x56,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _64M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 128MiB 1,8V 8-bit",
        .id = {0, 0x78,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _128M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 128MiB 1,8V 8-bit",
        .id = {0, 0x39,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _128M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 128MiB 3,3V 8-bit",
        .id = {0, 0x79,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _128M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 128MiB 1,8V 16-bit",
        .id = {0, 0x72,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _128M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 128MiB 1,8V 16-bit",
        .id = {0, 0x49,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _128M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 128MiB 3,3V 16-bit",
        .id = {0, 0x74,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _128M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 128MiB 3,3V 16-bit",
        .id = {0, 0x59,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _128M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 256MiB 3,3V 8-bit",
        .id = {0, 0x71,},
        .id_len = 2,
        .pagesize = _512B,
        .chipsize = _256M,
        .blocksize = _16K,
    },
    {
        .name = "NAND 64MiB 1,8V 8-bit",
        .id = {0, 0xA2,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _64M,
        .blocksize = 0,
    },
    {
        .name = "NAND 64MiB 3,3V 8-bit",
        .id = {0, 0xF2,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _64M,
        .blocksize = 0,
    },
    {
        .name = "NAND 64MiB 1,8V 16-bit",
        .id = {0, 0xB2,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _64M,
        .blocksize = 0,
    },
    {
        .name = "NAND 64MiB 3,3V 16-bit",
        .id = {0, 0xC2,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _64M,
        .blocksize = 0,
    },
    {
        .name = "NAND 128MiB 1,8V 8-bit",
        .id = {0, 0xA1,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _128M,
        .blocksize = 0,
    },
    {
        .name = "NAND 128MiB 3,3V 8-bit",
        .id = {0, 0xF1,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _128M,
        .blocksize = 0,
    },
    {
        .name = "NAND 128MiB 3,3V 8-bit",
        .id = {0, 0xD1,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _128M,
        .blocksize = 0,
    },
    {
        .name = "NAND 128MiB 1,8V 16-bit",
        .id = {0, 0xB1,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _128M,
        .blocksize = 0,
    },
    {
        .name = "NAND 128MiB 3,3V 16-bit",
        .id = {0, 0xC1,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _128M,
        .blocksize = 0,
    },
    {
        .name = "NAND 256MiB 1,8V 8-bit",
        .id = {0, 0xAA,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _256M,
        .blocksize = 0,
    },
    {
        .name = "NAND 256MiB 3,3V 8-bit",
        .id = {0, 0xDA,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _256M,
        .blocksize = 0,
    },
    {
        .name = "NAND 256MiB 1,8V 16-bit",
        .id = {0, 0xBA,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _256M,
        .blocksize = 0,
    },
    {
        .name = "NAND 256MiB 3,3V 16-bit",
        .id = {0, 0xCA,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _256M,
        .blocksize = 0,
    },
    {
        .name = "NAND 512MiB 1,8V 8-bit",
        .id = {0, 0xAC,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _512M,
        .blocksize = 0,
    },
    {
        .name = "NAND 512MiB 3,3V 8-bit",
        .id = {0, 0xDC,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _512M,
        .blocksize = 0,
    },
    {
        .name = "NAND 512MiB 1,8V 16-bit",
        .id = {0, 0xBC,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _512M,
        .blocksize = 0,
    },
    {
        .name = "NAND 512MiB 3,3V 16-bit",
        .id = {0, 0xCC,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _512M,
        .blocksize = 0,
    },
    {
        .name = "NAND 1GiB 1,8V 8-bit",
        .id = {0, 0xA3,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _1G,
        .blocksize = 0,
    },
    {
        .name = "NAND 1GiB 3,3V 8-bit",
        .id = {0, 0xD3,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _1G,
        .blocksize = 0,
    },
    {
        .name = "NAND 1GiB 1,8V 16-bit",
        .id = {0, 0xB3,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _1G,
        .blocksize = 0,
    },
    {
        .name = "NAND 1GiB 3,3V 16-bit",
        .id = {0, 0xC3,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _1G,
        .blocksize = 0,
    },
    {
        .name = "NAND 2GiB 1,8V 8-bit",
        .id = {0, 0xA5,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _2G,
        .blocksize = 0,
    },
    {
        .name = "NAND 2GiB 3,3V 8-bit",
        .id = {0, 0xD5,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _2G,
        .blocksize = 0,
    },
    {
        .name = "NAND 2GiB 1,8V 16-bit",
        .id = {0, 0xB5,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _2G,
        .blocksize = 0,
    },
    {
        .name = "NAND 2GiB 3,3V 16-bit",
        .id = {0, 0xC5,},
        .id_len = 2,
        .pagesize = 0,
        .chipsize = _2G,
        .blocksize = 0,
    },
    {NULL,{0},0,0,0,0,0,0,0},
};

/*---------------------------------------------------------------------------*/
/* nand_get_dev_info_by_id - [DEFAULT] Get Nand device information by ID */
/*---------------------------------------------------------------------------*/
struct nand_dev_info *nand_get_dev_info_by_id(struct nand_info *nand)
{
    char *id = nand->dev.id;
    struct nand_dev_info *find = &(nand->dev);
    struct nand_flash_info *cur;
    MTD_PR(INIT_DBG, "\t *-Start find Nand flash\n");

    INFO_MSG("Nand ID:");
    for(int i=0; i<NAND_MAX_ID_LEN; i++)
        INFO_MSG("0x%02X ",id[i]);
    INFO_MSG("\n");

    for (cur = nand_flash_info_t; cur->id_len; cur++) {
        if (cur->id[0] && memcmp(id,cur->id,cur->id_len))
            continue;

        if ((cur->id_len == 2) && (id[1] != cur->id[1]))
            continue;
        MTD_PR(INIT_DBG, "\t *-Found Nand: %s\n", cur->name);

        find->name = cur->name;
        find->id_len = cur->id_len;
        find->chipsize = cur->chipsize;
        find->pagesize = cur->pagesize;

        if (find->pagesize) {
            find->oobsize = cur->oobsize;
            find->blocksize = cur->blocksize;
        } else {
            /* The ID[3] include pagesize oobsize blocksize */
            unsigned char extid = id[3];

            /* Calc pagesize */
            find->pagesize = 1024 << (extid & 0x3);

            /* Calc oobsize */
            extid >>= 2;
            find->oobsize = (8 << (extid & 0x1)) \
                    * (find->pagesize >> 9);

            /* Calc blocksize */
            extid >>= 2;
            find->blocksize = 64 << (10 + (extid & 0x3));
        }

        find->page_shift = ffs(find->pagesize) - 1;
        find->pagemask = find->pagesize -1;
        find->block_shift = ffs(find->blocksize) - 1;
        find->blockmask = find->blocksize -1;
        if (find->chipsize & 0xffffffff)
            find->chip_shift = ffs((unsigned)find->chipsize) - 1;
        else
            find->chip_shift = ffs((unsigned)(find->chipsize >> 32))
                        + 31;

        MTD_PR(INIT_DBG, "\t *-End of found  nand flash\n");

        INFO_MSG("Nand:\"%s\"\n", nand->dev.name);
        INFO_MSG("Size:%sB ", ulltostr(nand->dev.chipsize));
        INFO_MSG("Block:%sB ", ulltostr(nand->dev.blocksize));
        INFO_MSG("Page:%sB ", ulltostr(nand->dev.pagesize));
        INFO_MSG("Oob:%sB ", ulltostr(nand->dev.oobsize));

        return find;
    }

    ERR_MSG("Not found  nand flash!!!\n");

    return NULL;
}

