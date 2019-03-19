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
#include "spi_nand_ids.h"

#include "hifmc_common.h"
#include "hifmc100.h"

/*---------------------------------------------------------------------------*/
static struct nand_host *nand_host;
static struct hifmc_host *hifmc_host;
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

    reg = FMC_CMD_CMD1(SPI_CMD_RESET);
    reg_write(nand_host, reg, FMC_CMD);

    reg = OP_CFG_FM_CS(nand->cur_cs);
    reg_write(nand_host, reg, FMC_OP_CFG);

    reg = FMC_OP_CMD1_EN(ENABLE) | FMC_OP_REG_OP_START;
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
    struct hifmc_host *hifmc_host = nand_host->priv;
    struct spi *spi = hifmc_host->spi;

    MTD_PR(ER_DBG, "\t*-Start send cmd erase!\n");

    reg = spi->driver->wait_ready(spi);
    if (reg) {
        ERR_MSG(" Erase wait ready fail! status: %#x\n", reg);
        return NAND_STATUS_FAIL;
    }

    reg = spi->driver->write_enable(spi);
    if (reg) {
        ERR_MSG(" Erase write enable failed! reg: %#x\n", reg);
        return NAND_STATUS_FAIL;
    }

    hifmc_host->set_system_clock(spi->erase->clock, ENABLE);

    reg = FMC_INT_CLR_ALL;
    reg_write(nand_host, reg, FMC_INT_CLR);

    reg = spi->erase->cmd;
    reg_write(nand_host, FMC_CMD_CMD1(reg), FMC_CMD);

    reg_write(nand_host, page, FMC_ADDRL);

    reg = OP_CFG_FM_CS(nand->cur_cs)
        | OP_CFG_MEM_IF_TYPE(spi->erase->iftype)
        | OP_CFG_ADDR_NUM(STD_OP_ADDR_NUM)
        | OP_CFG_DUMMY_NUM(spi->erase->dummy);
    reg_write(nand_host, reg, FMC_OP_CFG);

    reg = FMC_OP_CMD1_EN(ENABLE)
        | FMC_OP_ADDR_EN(ENABLE)
        //| FMC_OP_READ_STATUS_EN(ENABLE)
        | FMC_OP_REG_OP_START;
    reg_write(nand_host, reg, FMC_OP);

    FMC_CMD_WAIT_CPU_FINISH(nand_host);

    ret = nand->feature_op(nand, GET_OP, STATUS_ADDR, 0);
    MTD_PR(ER_DBG, "\t*-End send cmd erase ret=0x%0x!\n", ret);
    if (ret& SPINAND_ERASE_FAIL)
        ret = NAND_STATUS_FAIL;
    else
        ret = 0;

    return  ret;
}

/*---------------------------------------------------------------------------*/
/* nand page program */
/*---------------------------------------------------------------------------*/
static int hifmc100_program(struct nand_info *nand, uint32_t page)
{
    unsigned int reg, len;
    struct nand_host *nand_host = nand->priv;
    struct hifmc_host *hifmc_host = nand_host->priv;
    struct spi *spi = hifmc_host->spi;

#ifdef HIFMC100_SPI_NAND_SUPPORT_REG_WRITE
    const char *op = "Reg";
#else
    const char *op = "Dma";
#endif

    MTD_PR(WR_DBG, "*-Enter %s page program!\n", op);

    reg = spi->driver->wait_ready(spi);
    if (reg) {
        ERR_MSG(" %s program wait ready failed! status: %#x\n",
                op, reg);
        return NAND_STATUS_FAIL;
    }

    reg = spi->driver->write_enable(spi);
    if (reg) {
        ERR_MSG(" %s program write enable failed! reg: %#x\n",
                op, reg);
        return NAND_STATUS_FAIL;
    }

    hifmc_host->set_system_clock(spi->write->clock, ENABLE);

    reg = FMC_INT_CLR_ALL;
    reg_write(nand_host, reg, FMC_INT_CLR);

    reg = OP_CFG_FM_CS(nand->cur_cs)
        | OP_CFG_MEM_IF_TYPE(spi->write->iftype);
    reg_write(nand_host, reg, FMC_OP_CFG);

    reg = (page >> 16) & 0xff;
    reg_write(nand_host, reg, FMC_ADDRH);

    reg = (page & 0xffff) << 16;
    reg_write(nand_host, reg, FMC_ADDRL);

#ifndef HIFMC100_SPI_NAND_SUPPORT_REG_WRITE
    len = nand_host->pagesize + nand_host->oobsize;
    mtd_dma_cache_clean((void *)nand_host->dma_data, len);
    reg = nand_host->dma_data;
    reg_write(nand_host, reg, FMC_DMA_SADDR_D0);

    reg = nand_host->dma_oob;
    reg_write(nand_host, reg, FMC_DMA_SADDR_OOB);
#endif

    reg = OP_CTRL_WR_OPCODE(spi->write->cmd)
#ifdef HIFMC100_SPI_NAND_SUPPORT_REG_WRITE
        | OP_CTRL_DMA_OP(OP_TYPE_REG)
#else
        | OP_CTRL_DMA_OP(OP_TYPE_DMA)
#endif
        | OP_CTRL_RW_OP(RW_OP_WRITE)
        | OP_CTRL_DMA_OP_READY;
    reg_write(nand_host, reg, FMC_OP_CTRL);

    FMC_DMA_WAIT_INT_FINISH(nand_host);

    MTD_PR(WR_DBG, "*-End %s page program!\n", op);

    return 0;
}

/*---------------------------------------------------------------------------*/
/* hifmc100_read - Send comand to read nand devie data to buf */
/*---------------------------------------------------------------------------*/
static int hifmc100_read(struct nand_info *nand, unsigned int page)
{
    unsigned int reg, len;
    struct nand_host *nand_host = nand->priv;
    struct hifmc_host *hifmc_host = nand_host->priv;
    struct spi *spi = hifmc_host->spi;

#ifdef HIFMC100_SPI_NAND_SUPPORT_REG_READ
    char *op = "Reg";
#else
    char *op = "Dma";
#endif

    MTD_PR(RD_DBG, "\t*-Start %s page read\n", op);

    reg = spi->driver->wait_ready(spi);
    if (reg) {
        ERR_MSG(" %s read wait ready fail! reg: %#x\n", op, reg);
        return NAND_STATUS_FAIL;
    }

    hifmc_host->set_system_clock(spi->read->clock, ENABLE);

    reg = FMC_INT_CLR_ALL;
    reg_write(nand_host, reg, FMC_INT_CLR);

    reg = OP_CFG_FM_CS(nand->cur_cs)
        | OP_CFG_MEM_IF_TYPE(spi->read->iftype)
        | OP_CFG_DUMMY_NUM(spi->read->dummy);
    reg_write(nand_host, reg, FMC_OP_CFG);

    reg = (page >> 16) & 0xff;
    reg_write(nand_host, reg, FMC_ADDRH);

    reg = (page & 0xffff) << 16;
    reg_write(nand_host, reg, FMC_ADDRL);

#ifndef HIFMC100_SPI_NAND_SUPPORT_REG_READ
    len = nand_host->pagesize + nand_host->oobsize;
    mtd_dma_cache_inv((void *)nand_host->dma_data, len);
    reg = nand_host->dma_data;
    reg_write(nand_host, reg, FMC_DMA_SADDR_D0);

    reg = nand_host->dma_oob;
    reg_write(nand_host, reg, FMC_DMA_SADDR_OOB);
#endif

    reg = OP_CTRL_RD_OPCODE(spi->read->cmd)
#ifdef HIFMC100_SPI_NAND_SUPPORT_REG_READ
        | OP_CTRL_DMA_OP(OP_TYPE_REG)
#else
        | OP_CTRL_DMA_OP(OP_TYPE_DMA)
#endif
        | OP_CTRL_RW_OP(RW_OP_READ) | OP_CTRL_DMA_OP_READY;
    reg_write(nand_host, reg, FMC_OP_CTRL);

    FMC_DMA_WAIT_INT_FINISH(nand_host);

    MTD_PR(RD_DBG, "\t*-End %s page read\n", op);

    return 0;
}

/*---------------------------------------------------------------------------*/
/* hifmc100_ecc0_switch - config ecc mode */
/*---------------------------------------------------------------------------*/
static void hifmc100_ecc0_switch(struct nand_host *nand_host, unsigned char op)
{
    unsigned int config;

    if (op == ENABLE)
        config = nand_host->cfg_ecc0;
    else if (op == DISABLE)
        config = nand_host->cfg;
    else {
        ERR_MSG(" Invalid opcode: %d\n", op);
        return;
    }

    reg_write(nand_host, config, FMC_CFG);
}

/*---------------------------------------------------------------------------*/
/*    Send set/get features command to SPI Nand flash */
/*---------------------------------------------------------------------------*/
static uint8_t hifmc100_feature_op(struct nand_info *nand, uint8_t op,
        uint8_t addr, uint8_t val)
{
    unsigned int reg;
    struct nand_host *nand_host = nand->priv;
    const char *str[] = {"Get", "Set"};

    if ((GET_OP == op) && (STATUS_ADDR == addr)) {
        MTD_PR(SR_DBG, "\t\t|*-Start Get Status\n");

        reg = OP_CFG_FM_CS(nand->cur_cs);
        reg_write(nand_host, reg, FMC_OP_CFG);

        reg = FMC_OP_READ_STATUS_EN(ENABLE) | FMC_OP_REG_OP_START;
        reg_write(nand_host, reg, FMC_OP);

        FMC_CMD_WAIT_CPU_FINISH(nand_host);

        val = reg_read(nand_host, FMC_STATUS);
        MTD_PR(SR_DBG, "\t\t|*-End Get Status, result: %#x\n", val);

        return val;
    }

    MTD_PR(FT_DBG, "\t|||*-Start %s feature, addr[%#x]\n", str[op], addr);
    hifmc100_ecc0_switch(nand_host, ENABLE);

    reg = FMC_CMD_CMD1(op ? SPI_CMD_SET_FEATURE : SPI_CMD_GET_FEATURES);
    reg_write(nand_host, reg, FMC_CMD);

    reg_write(nand_host, addr, FMC_ADDRL);

    reg = OP_CFG_FM_CS(nand->cur_cs)
        | OP_CFG_ADDR_NUM(FEATURES_OP_ADDR_NUM);
    reg_write(nand_host, reg, FMC_OP_CFG);

    reg = FMC_DATA_NUM_CNT(FEATURES_DATA_LEN);
    reg_write(nand_host, reg, FMC_DATA_NUM);

    reg = FMC_OP_CMD1_EN(ENABLE)
        | FMC_OP_ADDR_EN(ENABLE)
        | FMC_OP_REG_OP_START;

    if (SET_OP == op) {
        reg |= FMC_OP_WRITE_DATA_EN(ENABLE);
        writeb(val, nand_host->membase);
    } else {
        reg |= FMC_OP_READ_DATA_EN(ENABLE);
    }
    reg_write(nand_host, reg, FMC_OP);

    FMC_CMD_WAIT_CPU_FINISH(nand_host);

    if (GET_OP == op) {
        val = readb(nand_host->membase);
    }

    hifmc100_ecc0_switch(nand_host, DISABLE);

    MTD_PR(FT_DBG, "\t|||*-End %s Feature[%#x]:%#x\n", str[op], addr, val);

    return val;
}

/*---------------------------------------------------------------------------*/
/* hifmc100_read_status - Send command to read nand device status */
/*---------------------------------------------------------------------------*/
static unsigned char hifmc100_read_status(struct nand_info *nand)
{
    unsigned int reg;

    reg = nand->feature_op(nand, GET_OP, STATUS_ADDR, 0);
    MTD_PR(RD_DBG, "Now SPI NAND FLASH status is 0x%02x.\n", reg);

    reg = reg & 0xff;
    switch(nand->op_state) {
        case NAND_PROGING:
            reg = (reg & SPINAND_PROG_FAIL) ? NAND_STATUS_FAIL : 0;
            break;
        case NAND_ERASING:;
            reg = (reg & SPINAND_ERASE_FAIL) ? NAND_STATUS_FAIL : 0;
            break;
        default:
            reg = 0;;
    }
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

    memset((char *)(nand_host->membase), 0, MAX_SPI_NAND_ID_LEN);

    hifmc100_ecc0_switch(nand_host, ENABLE);

    reg = FMC_CMD_CMD1(SPI_CMD_RDID);
    reg_write(nand_host, reg, FMC_CMD);

    reg = READ_ID_ADDR;
    reg_write(nand_host, reg, FMC_ADDRL);

    reg = OP_CFG_FM_CS(nand->cur_cs)
        | OP_CFG_ADDR_NUM(READ_ID_ADDR_NUM);
    reg_write(nand_host, reg, FMC_OP_CFG);

    reg = FMC_DATA_NUM_CNT(MAX_SPI_NAND_ID_LEN);
    reg_write(nand_host, reg, FMC_DATA_NUM);

    reg = FMC_OP_CMD1_EN(ENABLE)
        | FMC_OP_ADDR_EN(ENABLE)
        | FMC_OP_READ_DATA_EN(ENABLE)
        | FMC_OP_REG_OP_START;
    reg_write(nand_host, reg, FMC_OP);

    FMC_CMD_WAIT_CPU_FINISH(nand_host);

    memcpy((void *)id, (void *)nand_host->membase, MAX_SPI_NAND_ID_LEN);

    hifmc100_ecc0_switch(nand_host, DISABLE);

    MTD_PR(INIT_DBG, "\t *-End read flash ID\n");

}

/*---------------------------------------------------------------------------*/
/* ecc/pagesize get from NAND controller */
/*---------------------------------------------------------------------------*/
static struct nand_config_info hifmc100_spinand_config_table[] = {
    {NAND_PAGE_4K, NAND_ECC_24BIT_1K,   200, 1},
    {NAND_PAGE_4K, NAND_ECC_16BIT_1K,   144, 1},
    {NAND_PAGE_4K, NAND_ECC_8BIT_1K,    88,  1},
    {NAND_PAGE_4K, NAND_ECC_0BIT,       32,  1},

    {NAND_PAGE_2K, NAND_ECC_24BIT_1K,   128, 1},
    {NAND_PAGE_2K, NAND_ECC_16BIT_1K,   88,  1},
    {NAND_PAGE_2K, NAND_ECC_8BIT_1K,    64,  1},
    {NAND_PAGE_2K, NAND_ECC_0BIT,       32,  1},

    {0,            0,                   0,   0},
};

/*---------------------------------------------------------------------------*/
/* hifmc100_get_best_ecc - get best ecc config for nand */
/*---------------------------------------------------------------------------*/
static struct nand_config_info *hifmc100_get_best_ecc(struct nand_info *nand)
{
    struct nand_config_info *best = NULL;
    struct nand_config_info *info = hifmc100_spinand_config_table;

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

    nand_host->cfg_ecc0 = (nand_host->cfg & ~ECC_TYPE_MASK);

    MTD_PR(INIT_DBG, "HOST regbase=0x%08x.\n", nand_host->regbase);
    MTD_PR(INIT_DBG, "HOST membase=0x%08x.\n", nand_host->membase);
    MTD_PR(INIT_DBG, "HOST buforg=0x%0x.\n", nand_host->buforg);
    MTD_PR(INIT_DBG, "HOST buffer=0x%0x.\n", nand_host->buffer);
    MTD_PR(INIT_DBG, "HOST dma_oob=0x%0x.\n", nand_host->dma_oob);
    MTD_PR(INIT_DBG, "HOST dma_data=0x%0x.\n", nand_host->dma_data);
    MTD_PR(INIT_DBG, "HOST bbm=0x%0x.\n", nand_host->bbm);
    MTD_PR(INIT_DBG, "HOST cfg=0x%0x.\n", nand_host->cfg);
    MTD_PR(INIT_DBG, "HOST pagesize=0d%0d.\n", nand_host->pagesize);
    MTD_PR(INIT_DBG, "HOST oobsize=0d%0d.\n", nand_host->oobsize);
    MTD_PR(INIT_DBG, "HOST ecctype=0x%0x.\n", nand_host->ecctype);
    MTD_PR(INIT_DBG, "HOST version=0x%0x.\n", nand_host->version);

    INFO_MSG("Ecc:%s\n",nand_ecc_name(best->ecctype));
    return 0;
}

/*---------------------------------------------------------------------------*/
/*  Read status[C0H]:[0]bit OIP, judge whether the device is busy or not */
/*---------------------------------------------------------------------------*/
int spinand_general_wait_ready(struct spi *spi)
{
    unsigned char status;
    struct hifmc_host *hifmc_host = (struct hifmc_host *)(spi->host);
    struct nand_host *nand_host = (struct nand_host *)(hifmc_host->host);
    struct nand_info *nand = nand_host->nand;
    uint32_t timeout = 0xf0000000;

    do {
        status = nand->feature_op(nand, GET_OP, STATUS_ADDR, 0);
        if (!(status & STATUS_OIP_MASK)) {
            return (status & STATUS_OIP_MASK);
        }
        if(!timeout--)
            break;
    } while (1);
    ERR_MSG("SPI Nand wait ready timeout, status: %#x\n", status);
    return -1;
}

/*****************************************************************************/
/* Send set features cmd to SPI Nand, feature[B0H]:[0]bit QE would be set */
/*****************************************************************************/
int spinand_general_qe_enable(struct spi *spi)
{
    struct hifmc_host *hifmc_host = (struct hifmc_host *)(spi->host);
    struct nand_host *nand_host = (struct nand_host *)(hifmc_host->host);
    struct nand_info *nand = nand_host->nand;
    unsigned int reg, op;
    const char *str[] = {"Disable", "Enable"};

    MTD_PR(QE_DBG, "\t||*-Start SPI Nand flash QE\n");

    op = spi_is_quad(spi);

    MTD_PR(QE_DBG, "\t|||*-End Quad check, SPI Nand %s Quad.\n", str[op]);

    reg = nand->feature_op(nand, GET_OP, FEATURE_ADDR, 0);
    MTD_PR(QE_DBG, "\t|||-Get [%#x]feature: %#x\n", FEATURE_ADDR, reg);
    if ((reg & FEATURE_QE_ENABLE) == op) {
        MTD_PR(QE_DBG, "\t||*-SPI Nand quad was %sd!\n", str[op]);
        return op;
    }

    if (op == ENABLE)
        reg |= FEATURE_QE_ENABLE;
    else
        reg &= ~FEATURE_QE_ENABLE;

    nand->feature_op(nand, SET_OP, FEATURE_ADDR, reg);
    MTD_PR(QE_DBG, "\t|||-SPI Nand %s Quad\n", str[op]);

    spi->driver->wait_ready(spi);

    reg = nand->feature_op(nand, GET_OP, FEATURE_ADDR, 0);
    if ((reg & FEATURE_QE_ENABLE) == op)
        MTD_PR(QE_DBG, "\t|||-SPI Nand %s Quad succeed!\n", str[op]);
    else
        ERR_MSG(" %s Quad failed! reg: %#x\n", str[op], reg);

    MTD_PR(QE_DBG, "\t||*-End SPI Nand %s Quad.\n", str[op]);

    return op;
}

/****************************************************************************/
/* some spi nand flash don't QUAD enable */
/****************************************************************************/
int spinand_qe_not_enable(struct spi *spi)
{
    return 0;
}

/*---------------------------------------------------------------------------*/
/* Send write enable cmd to SPI Nand, status[C0H]:[2]bit WEL must be set 1 */
/*---------------------------------------------------------------------------*/
int spinand_general_write_enable(struct spi *spi)
{
    unsigned int reg;
    struct hifmc_host *hifmc_host = (struct hifmc_host *)spi->host;
    struct nand_host *nand_host = (struct nand_host *)hifmc_host->host;
    struct nand_info *nand = nand_host->nand;

    MTD_PR(WE_DBG, "\t|*-Start Write Enable\n");

    reg = nand->feature_op(nand, GET_OP, STATUS_ADDR, 0);
    if (reg & STATUS_WEL_MASK) {
        MTD_PR(WE_DBG, "\t||-Write Enable was opened! reg: %#x\n",
                reg);
        return 0;
    }

    reg = reg_read(nand_host, FMC_GLOBAL_CFG);
    if (reg & FMC_GLOBAL_CFG_WP_ENABLE) {
        reg &= ~FMC_GLOBAL_CFG_WP_ENABLE;
        reg_write(nand_host, reg, FMC_GLOBAL_CFG);
    }

    reg = FMC_CMD_CMD1(SPI_CMD_WREN);
    reg_write(nand_host, reg, FMC_CMD);

    reg = OP_CFG_FM_CS(nand->cur_cs);
    reg_write(nand_host, reg, FMC_OP_CFG);

    reg = FMC_OP_CMD1_EN(ENABLE) | FMC_OP_REG_OP_START;
    reg_write(nand_host, reg, FMC_OP);

    FMC_CMD_WAIT_CPU_FINISH(nand_host);

    MTD_PR(WE_DBG, "\t|*-End Write Enable\n");
    return 0;
}

/*****************************************************************************/
void hifmc100_spinand_op_map(struct spi *spi) {
    const int iftype_read[] = {
        SPI_IF_READ_STD,       SPI_IF_TYPE_STD,
        SPI_IF_READ_FAST,      SPI_IF_TYPE_STD,
        SPI_IF_READ_DUAL,      SPI_IF_TYPE_DUAL,
        SPI_IF_READ_DUAL_ADDR, SPI_IF_TYPE_DIO,
        SPI_IF_READ_QUAD,      SPI_IF_TYPE_QUAD,
        SPI_IF_READ_QUAD_ADDR, SPI_IF_TYPE_QIO,
        0, 0,
    };
    const int iftype_write[] = {
        SPI_IF_WRITE_STD,       SPI_IF_TYPE_STD,
        SPI_IF_WRITE_DUAL,      SPI_IF_TYPE_DUAL,
        SPI_IF_WRITE_DUAL_ADDR, SPI_IF_TYPE_DIO,
        SPI_IF_WRITE_QUAD,      SPI_IF_TYPE_QUAD,
        SPI_IF_WRITE_QUAD_ADDR, SPI_IF_TYPE_QIO,
        0, 0,
    };

    for (int ix = 0; iftype_write[ix]; ix += 2) {
        if (spi->write->iftype == iftype_write[ix]) {
            spi->write->iftype = iftype_write[ix + 1];
            break;
        }
    }

    for (int ix = 0; iftype_read[ix]; ix += 2) {
        if (spi->read->iftype == iftype_read[ix]) {
            spi->read->iftype = iftype_read[ix + 1];
            break;
        }
    }
    spi->erase->iftype = SPI_IF_TYPE_STD;
}

/*****************************************************************************/
static void hifmc100_ids_probe(struct nand_info *nand)
{
    unsigned reg;
    struct nand_host *nand_host = nand->priv;
    struct hifmc_host *hifmc_host = nand_host->priv;
    struct spi *spi = hifmc_host->spi;
    struct spi_nand_info *spi_dev = (struct spi_nand_info *)(nand->dev.priv);

    MTD_PR(INIT_DBG, "\t|*-Start match SPI operation & chip init\n");

    spi->host = hifmc_host;
    spi->name = spi_dev->name;
    spi->driver = spi_dev->driver;

    spi_nand_search_rw(spi_dev, spi->read,
            SPI_NAND_SUPPORT_READ,
            SPI_NAND_SUPPORT_MAX_DUMMY, RW_OP_READ);
    MTD_PR(INIT_DBG, "\t||-Save spi->read op cmd:%#x\n", spi->read->cmd);

    spi_nand_search_rw(spi_dev, spi->write,
            SPI_NAND_SUPPORT_WRITE,
            SPI_NAND_SUPPORT_MAX_DUMMY, RW_OP_WRITE);
    MTD_PR(INIT_DBG, "\t||-Save spi->write op cmd:%#x\n", spi->write->cmd);

    spi_nand_get_erase(spi_dev, spi->erase);
    MTD_PR(INIT_DBG, "\t||-Save spi->erase op cmd:%#x\n", spi->erase->cmd);

    hifmc100_spinand_op_map(spi);
    hifmc100_get_best_clock(&spi->write->clock);
    hifmc100_get_best_clock(&spi->read->clock);
    hifmc100_get_best_clock(&spi->erase->clock);

    spi->driver->qe_enable(spi);

    /* Disable write protection */
    reg = nand->feature_op(nand, GET_OP, PROTECT_ADDR, 0);
    MTD_PR(INIT_DBG, "\t||-Get protect status[%#x]: %#x\n", PROTECT_ADDR,
            reg);
    if (ANY_BP_ENABLE(reg)) {
        reg &= ~ALL_BP_MASK;
        nand->feature_op(nand, SET_OP, PROTECT_ADDR, reg);
        MTD_PR(INIT_DBG, "\t||-Set [%#x]FT %#x\n", PROTECT_ADDR, reg);

        spi->driver->wait_ready(spi);

        reg = nand->feature_op(nand, GET_OP, PROTECT_ADDR, 0);
        MTD_PR(INIT_DBG, "\t||-Check BP disable result: %#x\n", reg);
        if (ANY_BP_ENABLE(reg))
            ERR_MSG(" Write protection disable failed!\n");
    }

    /* Disable chip internal ECC */
    reg = nand->feature_op(nand, GET_OP, FEATURE_ADDR, 0);
    MTD_PR(INIT_DBG, "\t||-Get feature status[%#x]: %#x\n", FEATURE_ADDR,
            reg);
    if (reg & FEATURE_ECC_ENABLE) {
        reg &= ~FEATURE_ECC_ENABLE;
        nand->feature_op(nand, SET_OP, FEATURE_ADDR, reg);
        MTD_PR(INIT_DBG, "\t||-Set [%#x]FT: %#x\n", FEATURE_ADDR, reg);

        spi->driver->wait_ready(spi);

        reg = nand->feature_op(nand, GET_OP, FEATURE_ADDR, 0);
        MTD_PR(INIT_DBG, "\t||-Check internal ECC disable result: %#x\n",
                reg);
        if (reg & FEATURE_ECC_ENABLE)
            ERR_MSG(" Chip internal ECC disable failed!\n");
    }

    MTD_PR(INIT_DBG, "\t  *-End match SPI operation & chip init\n");
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
    nand->feature_op = hifmc100_feature_op;
    nand->ids_probe = hifmc100_ids_probe;

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
    MTD_PR(INIT_DBG, "\t  *-Start hifmc100 SPI Nand init\n");
    hifmc_host = zalloc(sizeof(struct hifmc_host));
    hifmc_host->host = nand_host;
    if(!hifmc_host) {
        ERR_MSG("no mem for hifmc_host!\n");
        return -1;
    }
    nand_host->priv = hifmc_host;

    nand_host->regbase = (void *)FMC_REG_BASE;
    nand_host->membase = (void *)FMC_MEM_BASE;
    reg = reg_read(nand_host, FMC_CFG);
    flash_type = (reg & FLASH_SEL_MASK) >> FLASH_SEL_SHIFT;
    if (flash_type != FLASH_TYPE_SPI_NAND) {
        ERR_MSG("Flash type isn't SPI Nand!\n");
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
    if (reg & FMC_GLOBAL_CFG_WP_ENABLE) {
        reg &= ~FMC_GLOBAL_CFG_WP_ENABLE;
        reg_write(nand_host, reg, FMC_GLOBAL_CFG);
    }

    reg = TIMING_CFG_TCSH(CS_HOLD_TIME)
        | TIMING_CFG_TCSS(CS_SETUP_TIME)
        | TIMING_CFG_TSHSL(CS_DESELECT_TIME);
    reg_write(nand_host, reg, FMC_SPI_TIMING_CFG);

    reg = ALL_BURST_ENABLE;
    reg_write(nand_host, reg, FMC_DMA_AHB_CTRL);

    hifmc_host->set_system_clock = hifmc100_set_system_clock;

    hifmc_host->set_system_clock(0, ENABLE);

    MTD_PR(INIT_DBG, "\t  *-End SPI Nand host init\n");

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

