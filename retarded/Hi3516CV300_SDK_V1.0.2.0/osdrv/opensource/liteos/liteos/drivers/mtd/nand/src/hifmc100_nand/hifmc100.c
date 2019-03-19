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

/*---------------------------------------------------------------------------*/
#include "string.h"
#include "stdlib.h"
#include "errno.h"

#include "asm/platform.h"
#include "asm/io.h"

#include "hisoc/flash.h"

#include "nand.h"
#include "nand_common.h"
#include "host_common.h"
#include "parallel_nand_ids.h"

#include "hifmc_common.h"
#include "hifmc100.h"

/*---------------------------------------------------------------------------*/
static struct nand_host *nand_host;
/*---------------------------------------------------------------------------*/
/* hifmc100_ecc_randomizer_ctrl - config ecc mode */
/*---------------------------------------------------------------------------*/
static void hifmc100_ecc_randomizer_ctrl(struct nand_host *nand_host, int ecc_en,
        int randomizer_en)
{
    unsigned int old_reg, reg, change = 0;
    char *ecc_op = ecc_en ? "Quit" : "Enter";
    char *rand_op = randomizer_en ? "Enable" : "Disable";

    if (IS_RANDOMIZER(nand_host)) {
        reg = old_reg = reg_read(nand_host, FMC_GLOBAL_CFG);
        if (randomizer_en)
            reg |= FMC_GLOBAL_CFG_RANDOMIZER_EN;
        else
            reg &= ~FMC_GLOBAL_CFG_RANDOMIZER_EN;

        if (old_reg != reg) {
            MTD_PR(ECC_DBG, "\t |*-Start %s randomizer\n", rand_op);
            MTD_PR(ECC_DBG, "\t ||-Get global CFG[%#x]%#x\n",
                    FMC_GLOBAL_CFG, old_reg);
            reg_write(nand_host, reg, FMC_GLOBAL_CFG);
            change++;
        }
    }

    old_reg = reg_read(nand_host, FMC_CFG);
    reg = (ecc_en ? nand_host->cfg : nand_host->cfg_ecc0);

    if (old_reg != reg) {
        MTD_PR(ECC_DBG, "\t |%s-Start %s ECC0 mode\n", change ? "|":"*",
                ecc_op);
        MTD_PR(ECC_DBG, "\t ||-Get CFG[%#x]%#x\n", FMC_CFG, old_reg);
        reg_write(nand_host, reg, FMC_CFG);
        change++;
    }

    if (ECC_DBG && change)
        MTD_PR(ECC_DBG, "\t |*-End randomizer and ECC0 mode config\n");
}

/*---------------------------------------------------------------------------*/
/* read a byte */
/*---------------------------------------------------------------------------*/
static uint8_t hifmc100_read_byte(struct nand_info *nand, int offset)
{
    struct nand_host *nand_host = nand->priv;
    return readb((char *)(nand_host->membase) + offset);
}

/*---------------------------------------------------------------------------*/
/* read a word */
/*---------------------------------------------------------------------------*/
static uint16_t hifmc100_read_word(struct nand_info *nand, int offset)
{
    struct nand_host *nand_host = nand->priv;
    return readw(nand_host->buffer + offset);
}

/*---------------------------------------------------------------------------*/
/* write buffer */
/*---------------------------------------------------------------------------*/
static void hifmc100_write_buf(struct nand_info *nand, const char *buf,
                int len, int offset)
{
    struct nand_host *nand_host = nand->priv;
    memcpy((void *)(nand_host->buffer + offset), (void *)buf, len);
}

/*---------------------------------------------------------------------------*/
/* read buffer */
/*---------------------------------------------------------------------------*/
static void hifmc100_read_buf(struct nand_info *nand, const char *buf, int len,
                     int offset)
{
    struct nand_host *nand_host = nand->priv;
    memcpy((void *)buf, (void *)(nand_host->buffer + offset), len);
}

/*---------------------------------------------------------------------------*/
/* reset nand */
/*---------------------------------------------------------------------------*/
static void hifmc100_reset(struct nand_info *nand)
{
    unsigned int reg;
    struct nand_host *nand_host = nand->priv;

    MTD_PR(INIT_DBG, "\t *-Start send cmd reset\n");

    reg = FMC_CMD_CMD1(NAND_CMD_RESET);
    reg_write(nand_host, reg, FMC_CMD);

    reg = OP_CFG_FM_CS(nand->cur_cs);
    reg_write(nand_host, reg, FMC_OP_CFG);

    reg = FMC_OP_CMD1_EN(ENABLE)
        | FMC_OP_WAIT_READY_EN(ENABLE)
        | FMC_OP_REG_OP_START;
    reg_write(nand_host, reg, FMC_OP);

    FMC_CMD_WAIT_CPU_FINISH(nand_host);

    MTD_PR(INIT_DBG, "\t *-End send cmd reset\n");

}

/*---------------------------------------------------------------------------*/
/* nand block erase */
/*---------------------------------------------------------------------------*/
static uint8_t hifmc100_erase(struct nand_info *nand, uint32_t page)
{
    unsigned int reg;
    uint8_t ret;
    struct nand_host *nand_host = nand->priv;

    MTD_PR(ER_DBG, "\t *-Start send cmd erase\n");

    /* Don't case the read retry config */
    hifmc100_ecc_randomizer_ctrl(nand_host, DISABLE, DISABLE);

    reg = page;
    reg_write(nand_host, reg, FMC_ADDRL);

    reg = FMC_CMD_CMD2(NAND_CMD_ERASE2) | FMC_CMD_CMD1(NAND_CMD_ERASE1);
    reg_write(nand_host, reg, FMC_CMD);

    reg = OP_CFG_FM_CS(nand->cur_cs)
        | OP_CFG_ADDR_NUM(NAND_ERASE_ADDR_CYCLE);
    reg_write(nand_host, reg, FMC_OP_CFG);

    /* need to config WAIT_READY_EN */
    reg = FMC_OP_WAIT_READY_EN(ENABLE)
        | FMC_OP_CMD1_EN(ENABLE)
        | FMC_OP_CMD2_EN(ENABLE)
        | FMC_OP_ADDR_EN(ENABLE)
        | FMC_OP_READ_STATUS_EN(ENABLE)
        | FMC_OP_REG_OP_START;
    reg_write(nand_host, reg, FMC_OP);

    FMC_CMD_WAIT_CPU_FINISH(nand_host);

    ret = ((reg_read(nand_host, FMC_STATUS) >> 8) & 0xff);
    MTD_PR(ER_DBG, "\t*-End send cmd erase ret=0x%0x!\n", ret);

    return  ret;
}

/*---------------------------------------------------------------------------*/
/* dma taransfer */
/*---------------------------------------------------------------------------*/
static void hifmc100_dma_transfer(struct nand_host *nand_host, int todev)
{
    struct nand_info *nand = nand_host->nand;
    char *op = todev ? "write" : "read";
    unsigned int reg, len;

    MTD_PR(DMA_DBG, "\t\t *-Start %s page dma transfer\n", op);
    len = nand_host->pagesize + nand_host->oobsize;

    if (nand_host->ecctype == NAND_ECC_0BIT) {
        reg = FMC_DMA_LEN_SET(nand_host->oobsize);
        reg_write(nand_host, reg, FMC_DMA_LEN);
        len = nand_host->oobsize;
    }
    reg = FMC_OP_READ_DATA_EN(ENABLE) | FMC_OP_WRITE_DATA_EN(ENABLE);
    reg_write(nand_host, reg, FMC_OP);


    reg = OP_CFG_FM_CS(nand->cur_cs)
        | OP_CFG_ADDR_NUM(NAND_PROG_READ_ADDR_CYCLE);
    reg_write(nand_host, reg, FMC_OP_CFG);

    reg = OP_CTRL_DMA_OP_READY;
    if (todev) {
        reg |= OP_CTRL_RW_OP(todev);
        mtd_dma_cache_clean((void *)nand_host->dma_data, len);
    } else {
        mtd_dma_cache_inv((void *)nand_host->dma_data, len);
    }
    reg_write(nand_host, reg, FMC_OP_CTRL);

    FMC_DMA_WAIT_CPU_FINISH(nand_host);

    MTD_PR(DMA_DBG, "\t\t *-End %s page dma transfer\n", op);

    return;
}

/*---------------------------------------------------------------------------*/
/* nand page program */
/*---------------------------------------------------------------------------*/
static int hifmc100_program(struct nand_info *nand, uint32_t page)
{
    unsigned int reg;
    struct nand_host *nand_host = nand->priv;
    MTD_PR(WR_DBG, "*-Enter page program!\n");

    if (*nand_host->bbm != 0xFF && *nand_host->bbm != 0x00)
        WARN_MSG("Attempt to write an invalid bbm."
                "page: 0x%08x, mark: 0x%02x,\n", page, *nand_host->bbm);

    hifmc100_ecc_randomizer_ctrl(nand_host, ENABLE, ENABLE);

    reg = (page >> 16) & 0xff;
    reg_write(nand_host, reg, FMC_ADDRH);

    reg = (page & 0xffff) << 16;
    reg_write(nand_host, reg, FMC_ADDRL);

    reg = FMC_CMD_CMD2(NAND_CMD_PROG1) | FMC_CMD_CMD1(NAND_CMD_PROG0);
    reg_write(nand_host, reg, FMC_CMD);

    *nand_host->ebm = 0x0000;

    hifmc100_dma_transfer(nand_host, RW_OP_WRITE);

    MTD_PR(WR_DBG, "*-End page program!\n");

    return 0;
}

/*---------------------------------------------------------------------------*/
/* hifmc100_read - Send comand to read nand devie data to buf */
/*---------------------------------------------------------------------------*/
static int hifmc100_read(struct nand_info *nand, unsigned int page)
{
    unsigned int reg;
    struct nand_host *nand_host = nand->priv;

    MTD_PR(RD_DBG, "\t*-Start page read\n");

    hifmc100_ecc_randomizer_ctrl(nand_host, ENABLE, ENABLE);

    reg = FMC_INT_CLR_ALL;
    reg_write(nand_host, reg, FMC_INT_CLR);

    reg = nand_host->cfg;
    reg_write(nand_host, reg, FMC_CFG);

    reg = (page >> 16) & 0xff;
    reg_write(nand_host, reg, FMC_ADDRH);

    reg = (page & 0xffff) << 16;
    reg_write(nand_host, reg, FMC_ADDRL);

    reg = FMC_CMD_CMD2(NAND_CMD_READ1) | FMC_CMD_CMD1(NAND_CMD_READ0);
    reg_write(nand_host, reg, FMC_CMD);

    hifmc100_dma_transfer(nand_host, RW_OP_READ);
    MTD_PR(RD_DBG, "\t*-End page read\n");
    return 0;
}

/*---------------------------------------------------------------------------*/
/* hifmc100_read_status - Send command to read nand device status */
/*---------------------------------------------------------------------------*/
static unsigned char hifmc100_read_status(struct nand_info *nand)
{
    struct nand_host *nand_host = nand->priv;
    unsigned int reg;

    hifmc100_ecc_randomizer_ctrl(nand_host, DISABLE, DISABLE);

    reg = OP_CFG_FM_CS(nand->cur_cs);
    reg_write(nand_host, reg, FMC_OP_CFG);

    reg = FMC_OP_READ_STATUS_EN(ENABLE) | FMC_OP_REG_OP_START;
    reg_write(nand_host, reg, FMC_OP);

    FMC_CMD_WAIT_CPU_FINISH(nand_host);

    reg = (reg >> 8) & 0xff;
    MTD_PR(RD_DBG, "Now SPI NAND FLASH status is 0x%02x.\n", reg);

    hifmc100_ecc_randomizer_ctrl(nand_host, ENABLE, ENABLE);

    return reg;
}


/*---------------------------------------------------------------------------*/
/* hifmc100_read_id - Send command to read nand device ID */
/*---------------------------------------------------------------------------*/
static void hifmc100_read_id(struct nand_info *nand, char *id)
{
    unsigned int reg;
    struct nand_host *nand_host = nand->priv;

    MTD_PR(INIT_DBG, "\t *-Start send cmd read ID\n");

    memset((char *)(nand_host->membase), 0, NAND_MAX_ID_LEN);

    hifmc100_ecc_randomizer_ctrl(nand_host, DISABLE, DISABLE);

    reg = FMC_CMD_CMD1(NAND_CMD_READID);
    reg_write(nand_host, reg, FMC_CMD);

    reg = 0;
    reg_write(nand_host, reg, FMC_ADDRL);

    reg = OP_CFG_FM_CS(nand->cur_cs)
        | OP_CFG_ADDR_NUM(READ_ID_ADDR_NUM);
    reg_write(nand_host, reg, FMC_OP_CFG);

    reg = FMC_DATA_NUM_CNT(NAND_MAX_ID_LEN);
    reg_write(nand_host, reg, FMC_DATA_NUM);

    reg = FMC_OP_CMD1_EN(ENABLE)
        | FMC_OP_ADDR_EN(ENABLE)
        | FMC_OP_READ_DATA_EN(ENABLE)
        | FMC_OP_REG_OP_START;
    reg_write(nand_host, reg, FMC_OP);

    FMC_CMD_WAIT_CPU_FINISH(nand_host);

    memcpy((void *)id, (void *)nand_host->membase, NAND_MAX_ID_LEN);

    hifmc100_ecc_randomizer_ctrl(nand_host,ENABLE, ENABLE);

    MTD_PR(INIT_DBG, "\t *-End read flash ID\n");

}

/*---------------------------------------------------------------------------*/
/* ecc/pagesize get from NAND controller */
/*---------------------------------------------------------------------------*/
static struct nand_config_info hifmc100_nand_config_table[] = {
    {NAND_PAGE_16K, NAND_ECC_64BIT_1K,  1824,   1},
    {NAND_PAGE_16K, NAND_ECC_40BIT_1K,  1200,   1},
    {NAND_PAGE_16K, NAND_ECC_0BIT,      32,     1},

    {NAND_PAGE_8K,  NAND_ECC_64BIT_1K,  928,    1},
    {NAND_PAGE_8K,  NAND_ECC_40BIT_1K,  600,    1},
    {NAND_PAGE_8K,  NAND_ECC_24BIT_1K,  368,    1},
    {NAND_PAGE_8K,  NAND_ECC_0BIT,      32,     1},

    {NAND_PAGE_4K,  NAND_ECC_24BIT_1K,  200,    1},
    //{NAND_PAGE_4K,  NAND_ECC_16BIT_1K,  128,    1},
    {NAND_PAGE_4K,  NAND_ECC_8BIT_1K,   128,    1},
    {NAND_PAGE_4K,  NAND_ECC_0BIT,      32,     1},

    {NAND_PAGE_2K,  NAND_ECC_24BIT_1K,  128,    1},
    //{NAND_PAGE_2K,  NAND_ECC_16BIT_1K,  64,     1},
    {NAND_PAGE_2K,  NAND_ECC_8BIT_1K,   64,     1},
    {NAND_PAGE_2K,  NAND_ECC_0BIT,      32,     1},

    {0,             0,                  0,      0},
};

/*---------------------------------------------------------------------------*/
/* hifmc100_get_best_ecc - get best ecc config for nand */
/*---------------------------------------------------------------------------*/
static struct nand_config_info *hifmc100_get_best_ecc(struct nand_info *nand)
{
    struct nand_config_info *best = NULL;
    struct nand_config_info *info = hifmc100_nand_config_table;

    for (; info->flag; info++) {
        if (nandpage_type2size(info->pagetype) != nand->dev.pagesize)
            continue;

        if (nand->dev.oobsize < info->oobsize)
            continue;

        if (!best || (best->ecctype < info->ecctype))
            best = info;
    }

    if (!best)
        ERR_MSG("Not support pagesize: %d and oobsize: %d.\n",
                nand->dev.pagesize, nand->dev.oobsize);
    return best;
}

/*---------------------------------------------------------------------------*/
/* hifmc100_oob_resize- nand oob resize */
/*---------------------------------------------------------------------------*/
static int hifmc100_oob_resize(struct nand_info *nand)
{
    unsigned char page_reg, pagetype, ecc_reg, ecctype;
    unsigned int reg;
    char *start_type = "unknown";
    struct nand_config_info *best = NULL;
    struct nand_host *nand_host = nand->priv;
    struct nand_status_info *nand_sts = &(nand->dev.status);

    MTD_PR(INIT_DBG, "\t *-Start match PageSize and EccType\n");

    best = hifmc100_get_best_ecc(nand);
    if (!best) {
        ERR_MSG("Can't find any configure for SPI Nand Flash!!!\n");
        return -1;
    }

    start_type = "Auto";

    pagetype = best->pagetype;
    ecctype = best->ecctype;
    page_reg = hifmc_page_type2reg(pagetype);
    reg = reg_read(nand_host, FMC_CFG);
    reg &= ~PAGE_SIZE_MASK;
    reg |= FMC_CFG_PAGE_SIZE(page_reg);
    MTD_PR(INIT_DBG, "\t |-%s Config, PageSize %s EccType %s OobSize %d\n",
            start_type, nand_page_name(pagetype),
            nand_ecc_name(ecctype), best->oobsize);

    ecc_reg = hifmc_ecc_type2reg(ecctype);
    reg &= ~ECC_TYPE_MASK;
    reg |= FMC_CFG_ECC_TYPE(ecc_reg);
    MTD_PR(INIT_DBG, "\t |-%s Config best EccType: %s\n", start_type,
            nand_ecc_name(best->ecctype));

    reg_write(nand_host, reg, FMC_CFG);

    nand_host->cfg = reg;

    if (best->ecctype != NAND_ECC_0BIT)
        nand->dev.oobsize = nand_host->oobsize = best->oobsize;

    nand_sts->ecctype = nand_host->ecctype = best->ecctype;
    nand->dev.pagesize = nand_host->pagesize = nandpage_type2size(best->pagetype);
    nand_sts->start_type = start_type;

    nand_host->buforg = memalign(CACHE_ALIGNED_SIZE,
            ALIGN(nand_host->pagesize + nand_host->oobsize, CACHE_ALIGNED_SIZE));
    if (!nand_host->buforg) {
        ERR_MSG("Can't malloc memory for NAND driver.\n");
        return -1;
    }
    nand_host->buffer = (char *) nand_host->buforg;
    nand_host->dma_data = (unsigned int)nand_host->buffer;
    nand_host->dma_oob = nand_host->dma_data + nand_host->pagesize;
    nand_host->bbm = (char *)(nand_host->buffer + nand_host->pagesize
            + NAND_BAD_BLOCK_POS);
    nand_host->ebm = (char *)(nand_host->buffer + nand_host->pagesize
            + NAND_EMPTY_BLOCK_POS);
    nand_host->cfg_ecc0 = (nand_host->cfg & ~ECC_TYPE_MASK);

    reg = (unsigned int)nand_host->dma_data;

    reg_write(nand_host, reg, FMC_DMA_SADDR_D0);

    reg += FMC_DMA_ADDR_OFFSET;
    reg_write(nand_host, reg, FMC_DMA_SADDR_D1);

    reg += FMC_DMA_ADDR_OFFSET;
    reg_write(nand_host, reg, FMC_DMA_SADDR_D2);

    reg += FMC_DMA_ADDR_OFFSET;
    reg_write(nand_host, reg, FMC_DMA_SADDR_D3);

    reg = nand_host->dma_oob;
    reg_write(nand_host, reg, FMC_DMA_SADDR_OOB);
    reg = FMC_DMA_AHB_CTRL_DMA_PP_EN
        | FMC_DMA_AHB_CTRL_BURST16_EN
        | FMC_DMA_AHB_CTRL_BURST8_EN
        | FMC_DMA_AHB_CTRL_BURST4_EN;
    reg_write(nand_host, reg, FMC_DMA_AHB_CTRL);

    MTD_PR(INIT_DBG, "HOST regbase=0x%08x.\n", nand_host->regbase);
    MTD_PR(INIT_DBG, "HOST membase=0x%08x.\n", nand_host->membase);
    MTD_PR(INIT_DBG, "HOST buforg=0x%0x.\n", nand_host->buforg);
    MTD_PR(INIT_DBG, "HOST buffer=0x%0x.\n", nand_host->buffer);
    MTD_PR(INIT_DBG, "HOST dma_oob=0x%0x.\n", nand_host->dma_oob);
    MTD_PR(INIT_DBG, "HOST dma_data=0x%0x.\n", nand_host->dma_data);
    MTD_PR(INIT_DBG, "HOST bbm=0x%0x.\n", nand_host->bbm);
    MTD_PR(INIT_DBG, "HOST ebm=0x%0x.\n", nand_host->ebm);
    MTD_PR(INIT_DBG, "HOST cfg=0x%0x.\n", nand_host->cfg);
    MTD_PR(INIT_DBG, "HOST pagesize=0d%0d.\n", nand_host->pagesize);
    MTD_PR(INIT_DBG, "HOST oobsize=0d%0d.\n", nand_host->oobsize);
    MTD_PR(INIT_DBG, "HOST ecctype=0x%0x.\n", nand_host->ecctype);
    MTD_PR(INIT_DBG, "HOST version=0x%0x.\n", nand_host->version);

    INFO_MSG("Ecc:%s\n",nand_ecc_name(best->ecctype));

    return 0;
}

/*---------------------------------------------------------------------------*/
/* hifmc100_nand_init - nand info member init */
/*---------------------------------------------------------------------------*/
static void hifmc100_nand_init(struct nand_info *nand)
{
    nand->erase = hifmc100_erase;
    nand->program = hifmc100_program;
    nand->read = hifmc100_read;

    nand->read_id = hifmc100_read_id;
    nand->read_status = hifmc100_read_status;
    nand->reset = hifmc100_reset;

    nand->read_byte = hifmc100_read_byte;
    nand->read_word = hifmc100_read_word;
    nand->read_buf = hifmc100_read_buf;
    nand->write_buf = hifmc100_write_buf;

    nand->oob_resize = hifmc100_oob_resize;

    nand->cur_cs = HIFMC100_SPI_NAND_CS_NUM;
}

/*---------------------------------------------------------------*/
/* hifmc100_host_init - host member init */
/*---------------------------------------------------------------*/
static int hifmc100_host_init(struct nand_host *nand_host)
{
    int reg, flash_type;
    MTD_PR(INIT_DBG, "\t  *-Start hifmc100 parallel Nand init\n");

    nand_host->regbase = (void *)FMC_REG_BASE;
    nand_host->membase = (void *)FMC_MEM_BASE;
    reg = reg_read(nand_host, FMC_CFG);
    flash_type = (reg & FLASH_SEL_MASK) >> FLASH_SEL_SHIFT;
    if (flash_type != FLASH_TYPE_NAND) {
        ERR_MSG("Flash type isn't Parallel Nand!\n");
        return -ENODEV;
    }
    if ((reg & OP_MODE_MASK) == OP_MODE_BOOT) {
        reg |= FMC_CFG_OP_MODE(OP_MODE_NORMAL);
        MTD_PR(INIT_DBG, "\t  |-Controller enter normal mode\n");
    }

    reg_write(nand_host, reg, FMC_CFG);
    nand_host->cfg = reg;
    nand_host->cfg_ecc0 = reg & ~ECC_TYPE_MASK;

    reg = reg_read(nand_host, FMC_GLOBAL_CFG);
    //reg |= FMC_GLOBAL_CFG_EDO_EN;
    if (reg & FMC_GLOBAL_CFG_RANDOMIZER_EN) {
        nand_host->flags &= ~NAND_RANDOMIZER;
        MTD_PR(INIT_DBG, "\t |-Default disable randomizer\n");
        reg &= ~FMC_GLOBAL_CFG_RANDOMIZER_EN;
        reg_write(nand_host, reg, FMC_GLOBAL_CFG);
    }

    reg = PWIDTH_CFG_RW_HCNT(RW_H_WIDTH)
        | PWIDTH_CFG_R_LCNT(R_L_WIDTH)
        | PWIDTH_CFG_W_LCNT(W_L_WIDTH);
    reg_write(nand_host, reg, FMC_PND_PWIDTH_CFG);

    reg = ALL_BURST_ENABLE;
    reg_write(nand_host, reg, FMC_DMA_AHB_CTRL);

    hifmc100_nand_clk_enable(ENABLE);

    MTD_PR(INIT_DBG, "\t  *-End Nand host init\n");

    return 0;
}

/*---------------------------------------------------------------------------*/
/* nand_host_init - nand controller initializtion entry */
/*---------------------------------------------------------------------------*/
int nand_host_init(struct nand_info *nand)
{
    nand_host = zalloc(sizeof(struct nand_host));
    if(!nand_host) {
        ERR_MSG("no mem for nand_host!\n");
        return -1;
    }

    nand->priv = nand_host;
    hifmc100_nand_init(nand);

    nand_host->nand = nand;
    if(hifmc100_host_init(nand_host)) {
        free(nand_host);
        return -1;
    }
    return 0;
}

