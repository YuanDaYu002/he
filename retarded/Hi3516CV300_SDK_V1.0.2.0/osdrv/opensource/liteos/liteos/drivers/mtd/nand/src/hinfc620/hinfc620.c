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

#include "asm/io.h"
#include "errno.h"
#include "string.h"
#include "stdlib.h"
#include "asm/platform.h"

#include "hinfc620.h"
#include "hinfc620_match.h"

#include "hisoc/nand.h"

#include "host_common.h"

/*---------------------------------------------------------------------------*/
static struct nand_host *host;

/*---------------------------------------------------------------------------*/
/* read a byte */
/*---------------------------------------------------------------------------*/
static uint8_t hinfc620_read_byte(struct nand_info *nand, int offset)
{
    struct nand_host *host = nand->priv;
    return readb((char *)(host->membase) + offset);
}

/*---------------------------------------------------------------------------*/
/* read a word */
/*---------------------------------------------------------------------------*/
static uint16_t hinfc620_read_word(struct nand_info *nand, int offset)
{
    struct nand_host *host = nand->priv;
    return readw(host->buffer + offset);
}

/*---------------------------------------------------------------------------*/
/* write buffer */
/*---------------------------------------------------------------------------*/
static void hinfc620_write_buf(struct nand_info *nand, const char *buf,
        int len, int offset)
{
    struct nand_host *host = nand->priv;
    memcpy((void *)(host->buffer + offset), (void *)buf, len);
}

/*---------------------------------------------------------------------------*/
/* read buffer */
/*---------------------------------------------------------------------------*/
static void hinfc620_read_buf(struct nand_info *nand, const char *buf,
        int len, int offset)
{
    struct nand_host *host = nand->priv;
    memcpy((void *)buf, (void *)(host->buffer + offset), len);
}

/*---------------------------------------------------------------------------*/
/* reset nand */
/*---------------------------------------------------------------------------*/
static void hinfc620_reset(struct nand_info *nand)
{
    unsigned int regval;
    struct nand_host *host = nand->priv;

    reg_write(host, NAND_CMD_RESET, HINFC620_CMD);

    regval = HINFC620_OP_CMD1_EN
        | (((nand->cur_cs & HINFC620_OP_NF_CS_MASK)
                    << HINFC620_OP_NF_CS_SHIFT)
                | HINFC620_OP_WAIT_READY_EN);

    reg_write(host, regval, HINFC620_OP);

    HINFC620_WAIT_OP_FINISH(host);
}

/*---------------------------------------------------------------------------*/
/* hinfc620_read_status - Send command to read nand device status */
/*---------------------------------------------------------------------------*/
static uint8_t hinfc620_read_status(struct nand_info *nand)
{
    unsigned int reg;
    struct nand_host *host = nand->priv;

    reg = SET_READ_STATUS_CMD(NAND_CMD_STATUS);
    reg_write(host, reg, HINFC620_CMD);

    reg = HINFC620_OP_WAIT_READY_EN | HINFC620_OP_READ_STATUS_EN;
    reg_write(host, reg, HINFC620_OP);

    HINFC620_WAIT_OP_FINISH(host);

    reg = reg_read(host, HINFC620_STATUS);

    reg = (reg >> 8) & 0xff;
    MTD_PR(RD_DBG, "Now NAND flash status is 0x%02x.\n", reg);

    return (uint8_t)reg;
}

/*---------------------------------------------------------------------------*/
/* nand block erase */
/*---------------------------------------------------------------------------*/
static uint8_t hinfc620_erase(struct nand_info *nand, uint32_t page)
{
    unsigned int regval;
    uint8_t ret;
    struct nand_host *host = nand->priv;
    host->addr_cycle = NAND_ERASE_ADDR_CYCLE;

    reg_write(host, page, HINFC620_ADDRL);
    reg_write(host, (NAND_CMD_STATUS << 16)
            | (NAND_CMD_ERASE2 << 8)
            | NAND_CMD_ERASE1,
            HINFC620_CMD);

    regval =  HINFC620_OP_READ_STATUS_EN
                | HINFC620_OP_WAIT_READY_EN
                | HINFC620_OP_CMD2_EN
                | HINFC620_OP_CMD1_EN
                | HINFC620_OP_ADDR_EN
                | ((nand->cur_cs & HINFC620_OP_NF_CS_MASK)
                        << HINFC620_OP_NF_CS_SHIFT)
                | ((host->addr_cycle & HINFC620_OP_ADDR_CYCLE_MASK)
                << HINFC620_OP_ADDR_CYCLE_SHIFT);

    reg_write(host, regval, HINFC620_OP);

    HINFC620_WAIT_OP_FINISH(host);

    ret = (uint8_t)((reg_read(host, HINFC620_STATUS) >> 8) & 0xff);

    return  ret;
}

/*---------------------------------------------------------------------------*/
/* hinfc620_dma_transfer - dma read/program transfer function */
/*---------------------------------------------------------------------------*/
static void hinfc620_dma_transfer(struct nand_host *host, uint32_t page,
        unsigned char dma_wr_en)
{
    unsigned int len, reg;
    struct nand_info *nand = host->nand;

    reg = host->cfg;
    reg_write(host, reg, HINFC620_CON);

    if (dma_wr_en)
        reg = SET_ALL_CMD(NAND_CMD_PROG1, NAND_CMD_PROG0);
    else
        reg = SET_ALL_CMD(NAND_CMD_READ1, NAND_CMD_READ0);
    reg_write(host, reg, HINFC620_CMD);

    reg = (page & 0xffff) << 16;
    reg_write(host, reg, HINFC620_ADDRL);

    reg = (page >> 16) & 0xff;
    reg_write(host, reg, HINFC620_ADDRH);

    reg = host->dma_data;
    reg_write(host, reg, HINFC620_DMA_ADDR_DATA);

    if(host->pagesize == _8K) {
        reg = host->dma_data + HINFC620_DMA_ADDR_OFFSET;
        reg_write(host, reg, HINFC620_DMA_ADDR_DATA1);
    }

    if(host->pagesize == _16K) {
        reg += HINFC620_DMA_ADDR_OFFSET;
        reg_write(host, reg, HINFC620_DMA_ADDR_DATA2);

        reg += HINFC620_DMA_ADDR_OFFSET;
        reg_write(host, reg, HINFC620_DMA_ADDR_DATA3);
    }

    reg = host->dma_oob;
    reg_write(host, reg, HINFC620_DMA_ADDR_OOB);

    if (host->ecctype == NAND_ECC_0BIT) {
        reg = SET_DMA_OOB_LEN(host->oobsize);
        reg_write(host, reg, HINFC620_DMA_LEN);

        reg = HINFC620_DMA_PARA_DATA_RW_EN
            | HINFC620_DMA_PARA_OOB_RW_EN;
    } else {
        reg = HINFC620_DMA_PARA_DATA_RW_EN
            | HINFC620_DMA_PARA_OOB_RW_EN
            | HINFC620_DMA_PARA_DATA_EDC_EN
            | HINFC620_DMA_PARA_OOB_EDC_EN;
    }
    reg_write(host, reg, HINFC620_DMA_PARA);

        len = host->pagesize + host->oobsize;

    if (dma_wr_en)/* clean cache when write to nand device */
        mtd_dma_cache_clean((void *)host->dma_data, len);
    else/* Invalidate cache when read from nand device */
        mtd_dma_cache_inv((void *)host->dma_data, len);

    reg = SET_DMA_CTRL_CS(nand->cur_cs);
    reg &= ~DMA_CTRL_ADDR_NUM_MASK;
    reg |= (DMA_CTRL_BURST16_EN
            | DMA_CTRL_BURST8_EN
            | DMA_CTRL_BURST4_EN);
    if (dma_wr_en)
        reg |= HINFC620_DMA_CTRL_WE;
    else
        reg &= ~HINFC620_DMA_CTRL_WE;
    reg |= DMA_CTRL_DMA_START;
    reg_write(host, reg, HINFC620_DMA_CTRL);

    HINFC620_WAIT_DMA_FINISH(host);
}

/*---------------------------------------------------------------------------*/
/* nand page program */
/*---------------------------------------------------------------------------*/
static int hinfc620_program(struct nand_info *nand, uint32_t page)
{
    struct nand_host *host = nand->priv;

    if (*host->bbm != 0xFF && *host->bbm != 0x00)
        WARN_MSG("Attempt to write an invalid bbm. " \
               "page: 0x%08x, mark: 0x%02x,\n",
                page, *host->bbm);
    host->addr_cycle = NAND_PROG_READ_ADDR_CYCLE;
    hinfc620_dma_transfer(host, page, DMA_WR_TYPE_WRITE);

    return 0;
}

/*---------------------------------------------------------------------------*/
/* hinfc620_read - Send comand to read nand devie data to buf */
/*---------------------------------------------------------------------------*/
static int hinfc620_read(struct nand_info *nand, uint32_t page)
{
    struct nand_host *host = nand->priv;

    host->addr_cycle = NAND_PROG_READ_ADDR_CYCLE;
    hinfc620_dma_transfer(host, page, DMA_WR_TYPE_READ);

    return 0;
}

/*---------------------------------------------------------------------------*/
/* hinfc620_ecc_randomizer_ctrl - config ecc  and randomizer mode */
/*---------------------------------------------------------------------------*/
static void hinfc620_ecc_randomizer_ctrl(struct nand_host *host, int ecc_en,
        int randomizer_en)
{
    unsigned int nfc_con;

    if((!ecc_en) && randomizer_en) {
        ERR_MSG("NANDC620 don't support ecc0 when randomizer enable!\n");
        return;
    }

    if (IS_RANDOMIZER(host)) {
        if (randomizer_en)
            host->cfg |= HINFC620_CON_RANDOMIZER_EN;
        else
            host->cfg &= ~HINFC620_CON_RANDOMIZER_EN;
    }

    host->cfg_ecc0 &= ~HINFC620_CON_RANDOMIZER_EN;
    nfc_con = (ecc_en ? host->cfg : host->cfg_ecc0);
    reg_write(host, nfc_con, HINFC620_CON);
}

/*---------------------------------------------------------------------------*/
/* hinfc620_read_id - Send command to read nand device ID */
/*---------------------------------------------------------------------------*/
static void hinfc620_read_id(struct nand_info *nand, char *id)
{
    unsigned int reg;
    struct nand_host *host = nand->priv;

    /*use ecc0 to read id, and randomizer must be disable*/
    hinfc620_ecc_randomizer_ctrl(host, DISABLE, DISABLE);

    memset((void *)(host->membase), 0, NAND_MAX_ID_LEN);

    reg = NAND_MAX_ID_LEN;
    reg_write(host, reg, HINFC620_DATA_NUM);

    reg = NAND_CMD_READID;
    reg_write(host, reg, HINFC620_CMD);

    reg = 0;
    reg_write(host, reg, HINFC620_ADDRH);
    reg_write(host, reg, HINFC620_ADDRL);

    reg = HINFC620_READ_1CMD_1ADD_DATA;
    reg_write(host, reg, HINFC620_OP);

    HINFC620_WAIT_OP_FINISH(host);

    /*recovery ecctype and randomizer setting*/
    hinfc620_ecc_randomizer_ctrl(host, ENABLE, ENABLE);

    memcpy((void *)id, (void *)host->membase, NAND_MAX_ID_LEN);
}

/*---------------------------------------------------------------------------*/
/* ecc/pagesize config table */
/*---------------------------------------------------------------------------*/
static struct nand_config_info hinfc620_config_table[] = {
    {NAND_PAGE_16K, NAND_ECC_64BIT_1K, 1824},
    {NAND_PAGE_16K, NAND_ECC_40BIT_1K, 1200},
    {NAND_PAGE_16K, NAND_ECC_0BIT, 32},

    {NAND_PAGE_8K, NAND_ECC_64BIT_1K, 928},
    {NAND_PAGE_8K, NAND_ECC_40BIT_1K, 600},
    {NAND_PAGE_8K, NAND_ECC_24BIT_1K, 368},
    {NAND_PAGE_8K, NAND_ECC_0BIT, 32},

    {NAND_PAGE_4K, NAND_ECC_24BIT_1K, 200},
    {NAND_PAGE_4K, NAND_ECC_8BIT_1K, 128},
    {NAND_PAGE_4K, NAND_ECC_0BIT, 32},

    {NAND_PAGE_2K, NAND_ECC_24BIT_1K, 128},
    {NAND_PAGE_2K, NAND_ECC_8BIT_1K, 64},
    {NAND_PAGE_2K, NAND_ECC_0BIT, 32},

    {0, 0, 0},
};

/*---------------------------------------------------------------------------*/
/* hinfc620_get_best_ecc - get best ecc config for nand */
/*---------------------------------------------------------------------------*/
static struct nand_config_info *hinfc620_get_best_ecc(
        struct nand_config_info *config, struct nand_info *nand)
{
    struct nand_config_info *best = NULL;
    for (; config->oobsize; config++) {
        if (nandpage_type2size(config->pagetype)
                != nand->dev.pagesize)
            continue;

        if (nand->dev.oobsize < config->oobsize)
            continue;

        if (!best || (best->ecctype < config->ecctype))
            best = config;
    }

    if (!best) {
        ERR_MSG("Driver does not support the pagesize(%d) "
            "and oobsize(%d).\n",
            nand->dev.pagesize, nand->dev.oobsize);
    return NULL;
    }
    return best;
}

/*---------------------------------------------------------------------------*/
/* hinfc620_oob_resize- nand oob resize */
/*---------------------------------------------------------------------------*/
static int hinfc620_oob_resize(struct nand_info *nand)
{
    struct nand_config_info *best = NULL;
    struct nand_config_info *config = NULL;
    struct nand_host *host = nand->priv;
    struct nand_status_info *nand_sts = &(nand->dev.status);

    config = hinfc620_config_table;
    best = hinfc620_get_best_ecc(config, nand);
    if (!best) {
        ERR_MSG("Please configure Nand Flash pagesize and ecctype!\n");
        return -1;
    }

    host->oobsize = best->oobsize;

    nand_sts->flags |= host->flags;
    nand_sts->ecctype = host->ecctype = best->ecctype;
    if (IS_HW_AUTO(host))
        nand_sts->start_type = "HW-Auto";
    else
        nand_sts->start_type = "HW-Pin";

    host->pagesize = nand->dev.pagesize;
    host->buforg = memalign(CACHE_ALIGNED_SIZE,
            ALIGN(host->pagesize + host->oobsize, CACHE_ALIGNED_SIZE));
    if (!host->buforg) {
        ERR_MSG("no mem for dma area!\n");
        return -1;
    }
    host->buffer = (char *) host->buforg;
    host->dma_data = (unsigned int)host->buffer;

    memset((char *)host->membase, 0x0,
           HINFC620_BUFFER_BASE_ADDRESS_LEN);
    memset(host->buffer, 0x0,
        (host->pagesize + host->oobsize));

    host->dma_oob = host->dma_data + host->pagesize;
    host->bbm = (char *)(host->buffer
        + host->pagesize + NAND_BAD_BLOCK_POS);
    host->ebm = (char *)(host->buffer
        + host->pagesize + NAND_EMPTY_BLOCK_POS);

    host->cfg  = (HINFC620_CON_OP_MODE_NORMAL
        | ((hinfc620_page_type2reg(best->pagetype) &
            HINFC620_CON_PAGESIZE_MASK)
           << HINFC620_CON_PAGEISZE_SHIFT)
        | HINFC620_CON_READY_BUSY_SEL
        | ((hinfc620_ecc_type2reg(best->ecctype) &
            HINFC620_CON_ECCTYPE_MASK)
           << HINFC620_CON_ECCTYPE_SHIFT));

    if (host->pagesize >= _8K)//enable randomize for 8k/16k device
        host->cfg |= HINFC620_CON_RANDOMIZER_EN;

    host->cfg_ecc0 = (HINFC620_CON_OP_MODE_NORMAL
        | ((hinfc620_page_type2reg(best->pagetype) &
            HINFC620_CON_PAGESIZE_MASK)
           << HINFC620_CON_PAGEISZE_SHIFT)
        | HINFC620_CON_READY_BUSY_SEL);

    MTD_PR(INIT_DBG, "HOST regbase=0x%08x.\n", host->regbase);
    MTD_PR(INIT_DBG, "HOST buforg=0x%0x.\n", host->buforg);
    MTD_PR(INIT_DBG, "HOST buffer=0x%0x.\n", host->buffer);
    MTD_PR(INIT_DBG, "HOST dma_oob=0x%0x.\n", host->dma_oob);
    MTD_PR(INIT_DBG, "HOST dma_data=0x%0x.\n", host->dma_data);
    MTD_PR(INIT_DBG, "HOST bbm=0x%0x.\n", host->bbm);
    MTD_PR(INIT_DBG, "HOST cfg=0x%0x.\n", host->cfg);
    MTD_PR(INIT_DBG, "HOST pagesize=0d%0d.\n", host->pagesize);
    MTD_PR(INIT_DBG, "HOST oobsize=0d%0d.\n", host->oobsize);
    MTD_PR(INIT_DBG, "HOST ecctype=0x%0x.\n", host->ecctype);
    MTD_PR(INIT_DBG, "HOST version=0x%0x.\n", host->version);

    INFO_MSG("Ecc:%s\n",nand_ecc_name(best->ecctype));

    return 0;
}

/*---------------------------------------------------------------------------*/
/* hinfc620_nand_init - nand info member init */
/*---------------------------------------------------------------------------*/
static void hinfc620_nand_init(struct nand_info *nand)
{
    nand->erase = hinfc620_erase;
    nand->program = hinfc620_program;
    nand->read = hinfc620_read;

    nand->read_id = hinfc620_read_id;
    nand->read_status = hinfc620_read_status;
    nand->reset = hinfc620_reset;

    nand->read_byte = hinfc620_read_byte;
    nand->read_word = hinfc620_read_word;
    nand->read_buf = hinfc620_read_buf;
    nand->write_buf = hinfc620_write_buf;
    nand->oob_resize = hinfc620_oob_resize;

    nand->cur_cs = HINFC620_NAND_CS_NUM;
}

/*---------------------------------------------------------------*/
/* hinfc620_host_init - host member init */
/*---------------------------------------------------------------*/
static void hinfc620_host_init(struct nand_host *host)
{
    unsigned int regval;

    host->regbase = (void *)NANDC_REG_BASE;
    host->membase = (void *)NANDC_MEM_BASE;

    regval = reg_read(host, HINFC620_CON);
    if (regval & HINFC620_CON_RANDOMIZER_EN)
        host->flags |= NAND_RANDOMIZER;

    host->cfg = regval
            | HINFC620_CON_OP_MODE_NORMAL
            | HINFC620_CON_READY_BUSY_SEL;

    host->cfg_ecc0 = (host->cfg & (~HINFC620_CON_RANDOMIZER_EN)
            & (~(HINFC620_CON_ECCTYPE_MASK
                << HINFC620_CON_ECCTYPE_SHIFT)));

    regval = reg_read(host, HINFC620_BOOT_CFG);
    if (regval & HINFC620_BOOT_CFG_SAVE_PIN_MODE)
        host->flags |= NAND_HW_AUTO;

    if (regval & HINFC620_BOOT_CFG_SYC_NAND_PAD)
        host->flags |= NAND_SYNCHRONOUS_BOOT;

    /* check if start form nand or not */
    regval = check_boot_type();
    if (regval == BOOT_FROM_NAND)
        host->flags |= NAND_CONFIG_DONE;

    host->version = reg_read(host, HINFC620_VERSION);

    reg_write(host, (SET_HINFC620_PWIDTH(
            CONFIG_HINFC620_W_LATCH,
            CONFIG_HINFC620_R_LATCH,
            CONFIG_HINFC620_RW_LATCH)),
            HINFC620_PWIDTH);
    reg_write(host, CONFIG_HINFC620_OPIDLE, HINFC620_OPIDLE);
}

/*---------------------------------------------------------------------------*/
/* nand_host_init - nand controller initializtion entry */
/*---------------------------------------------------------------------------*/
int nand_host_init(struct nand_info *nand)
{
    host = zalloc(sizeof(struct nand_host));
    if(!host) {
        ERR_MSG("no mem for nand_host!\n");
        return -1;
    }

    nand->priv = host;
    hinfc620_nand_init(nand);

    host->nand = nand;
    hinfc620_host_init(host);
    hinfc620_clk_enable(ENABLE);

    return 0;
}

