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
#include "stdlib.h"
#include "string.h"
#include "asm/errno.h"

#include <linux/mtd/mtd.h>

#include "hisoc/flash.h"

#include "host_common.h"
#include "spinor_common.h"
#include "spi_common.h"

#include "hifmc_common.h"
#include "hifmc100.h"

#include "reset_shell.h"

static struct spinor_host *host;

/*---------------------------------------------------------------------------*/
/* hifmc100_read_id */
/*---------------------------------------------------------------------------*/
static void hifmc100_read_id(struct spinor_info *spinor, char *id)
{
    int reg;
    struct spinor_host *host = spinor->priv;

    reg = FMC_CMD_CMD1(SPI_CMD_RDID);
    reg_write(host, reg, FMC_CMD);

    reg = OP_CFG_FM_CS(spinor->cur_cs);
    reg_write(host, reg, FMC_OP_CFG);

    reg = FMC_DATA_NUM_CNT(SPI_NOR_MAX_ID_LEN);
    reg_write(host, reg, FMC_DATA_NUM);

    reg = FMC_OP_CMD1_EN(ENABLE)
        | FMC_OP_READ_DATA_EN(ENABLE)
        | FMC_OP_REG_OP_START;
    reg_write(host, reg, FMC_OP);

    FMC_CMD_WAIT_CPU_FINISH(host);

    memcpy(id, host->membase, SPI_NOR_MAX_ID_LEN);
}

/*---------------------------------------------------------------------------*/
/* hifmc100_dma_transfer */
/*---------------------------------------------------------------------------*/
static void hifmc100_dma_transfer(struct spinor_host *host,
        uint32_t spi_start_addr, char *dma_buffer,
        uint8_t rw, uint32_t size)
{
    uint8_t if_type = 0, dummy = 0;
    uint8_t w_cmd = 0, r_cmd = 0;
    int reg;
    struct spi *spi = host->spi;

    MTD_PR(DMA_DBG, "\t\t *-Start dma transfer => [%#x], len[%#x].\n",
            spi_start_addr, size);
    if (rw == WRITE)
        mtd_dma_cache_clean((void *)dma_buffer, size);
    else
        mtd_dma_cache_inv((void *)dma_buffer, size);

    reg = FMC_INT_CLR_ALL;
    reg_write(host, reg, FMC_INT_CLR);

    reg = spi_start_addr;
    reg_write(host, reg, FMC_ADDRL);

    if (rw == WRITE) {
        if_type = spi->write->iftype;
        dummy = spi->write->dummy;
        w_cmd = spi->write->cmd;
    } else {
        if_type = spi->read->iftype;
        dummy = spi->read->dummy;
        r_cmd = spi->read->cmd;
    }

    reg = OP_CFG_FM_CS(spi->cs)
        | OP_CFG_MEM_IF_TYPE(if_type)
        | OP_CFG_ADDR_NUM(spi->addrcycle)
        | OP_CFG_DUMMY_NUM(dummy);
    reg_write(host, reg, FMC_OP_CFG);

    reg = FMC_DMA_LEN_SET(size);
    reg_write(host, reg, FMC_DMA_LEN);

    reg = (uint32_t)dma_buffer;
    reg_write(host, reg, FMC_DMA_SADDR_D0);

    reg = OP_CTRL_RD_OPCODE(r_cmd)
        | OP_CTRL_WR_OPCODE(w_cmd)
        | OP_CTRL_RW_OP(rw)
        | OP_CTRL_DMA_OP_READY;
    reg_write(host, reg, FMC_OP_CTRL);

    FMC_DMA_WAIT_INT_FINISH(host);
    if (READ == rw) {
        mtd_dma_cache_inv((void *)dma_buffer, size);
    }
    return;
}

/*---------------------------------------------------------------------------*/
/* hifmc100_dma_read */
/*---------------------------------------------------------------------------*/
static int hifmc100_dma_read(struct spinor_host *host, uint32_t from,
        uint32_t len, const char *buf)
{
    int num, reg;
    struct spi *spi = host->spi;

    reg = spi->driver->wait_ready(spi);
    if (reg) {
        ERR_MSG(" Dma read wait ready fail! reg:%#x\n", reg);
        return -EBUSY;
    }

    host->set_system_clock(spi->read->clock, ENABLE);

    if ((unsigned)buf & HIFMC100_DMA_ALIGN_MASK) {
        num = HIFMC100_DMA_ALIGN_SIZE -
            ((unsigned)buf & (HIFMC100_DMA_ALIGN_MASK));
        if (num > len)
            num = len;
        hifmc100_dma_transfer(host, from,
                (char *)host->dma_buffer, READ, num);
        memcpy((void *)buf, (void *)host->dma_buffer, num);
        from  += num;
        buf += num;
        len -= num;
    }

    if ((num = len & (~HIFMC100_DMA_ALIGN_MASK))) {
        hifmc100_dma_transfer(host, from, (char *)buf, READ, num);
        buf  += num;
        from += num;
        len  -= num;
    }

    if (len) {
        num = len;
        hifmc100_dma_transfer(host, from,
                (char *)host->dma_buffer, READ, num);
        memcpy((void *)buf, (void *)host->dma_buffer, num);
        buf  += num;
        len  -= num;
    }

    return 0;
}

/*---------------------------------------------------------------------------*/
/* hifmc100_read */
/*---------------------------------------------------------------------------*/
static int hifmc100_read(struct spinor_info *spinor, uint32_t from,
        uint32_t len, const char *buf)
{
    int result;
    struct spinor_host *host = spinor->priv;

    MTD_PR(ER_DBG, "Start read buf=0x%08x from=0x%08x len=0x%08x.\n",
            buf, from, len);
    get_host(host);
    result = hifmc100_dma_read(host, from, len, buf);
    put_host(host);

    return result;
}

/*---------------------------------------------------------------------------*/
/* hifmc100_dma_write */
/*---------------------------------------------------------------------------*/
static int hifmc100_dma_write(struct spinor_host *host, uint32_t to,
        uint32_t len, const char *buf)
{
    int num, reg;
    struct spi *spi = host->spi;

    reg = spi->driver->wait_ready(spi);
    if (reg) {
        ERR_MSG("Dma write wait ready fail! reg:%#x\n", reg);
        return -EBUSY;
    }
    spi->driver->write_enable(spi);

    host->set_system_clock(spi->write->clock, ENABLE);

    if ((unsigned)buf & HIFMC100_DMA_ALIGN_MASK) {
        num = HIFMC100_DMA_ALIGN_SIZE -
            ((unsigned)buf & (HIFMC100_DMA_ALIGN_MASK));
        if (num > len)
            num = len;
        memcpy((void *)host->dma_buffer, (void *)buf, num);
        hifmc100_dma_transfer(host, to,
                (char *)host->dma_buffer, WRITE, num);

        to  += num;
        buf += num;
        len -= num;
    }

    if ((num = len & (~HIFMC100_DMA_ALIGN_MASK))) {
        hifmc100_dma_transfer(host, to, (char *)buf, WRITE, num);
        to  += num;
        buf += num;
        len -= num;
    }

    if (len) {
        num = len;
        memcpy((void *)host->dma_buffer, (void *)buf, num);
        hifmc100_dma_transfer(host, to,
                (char *)host->dma_buffer, WRITE, num);

        buf += num;
        len -= num;
    }

    return 0;
}

/*---------------------------------------------------------------------------*/
/* hifmc100_write */
/*---------------------------------------------------------------------------*/
static int hifmc100_write(struct spinor_info *spinor, uint32_t to, uint32_t len,
        const char *buf)
{
    int result;
    struct spinor_host *host = spinor->priv;

    MTD_PR(ER_DBG, "Start write buf=0x%08x to=0x%08x len=0x%08x.\n", buf, to, len);
    get_host(host);
    result = hifmc100_dma_write(host, to, len, buf);
    put_host(host);

    return result;
}

/*---------------------------------------------------------------------------*/
/* hifmc100_erase_one_block */
/*---------------------------------------------------------------------------*/
static int hifmc100_erase_one_block(struct spinor_host *host, uint32_t offset)
{
    int reg;
    struct spi *spi = host->spi;

    reg = spi->driver->wait_ready(spi);
    if (reg) {
        ERR_MSG("Erase wait ready fail! reg:%#x\n", reg);
        return -EBUSY;
    }
    spi->driver->write_enable(spi);

    host->set_system_clock(spi->erase->clock, ENABLE);

    reg = FMC_CMD_CMD1(spi->erase->cmd);
    reg_write(host, reg, FMC_CMD);

    reg = offset;
    reg_write(host, reg, FMC_ADDRL);

    reg = OP_CFG_FM_CS(spi->cs)
        | OP_CFG_MEM_IF_TYPE(spi->erase->iftype)
        | OP_CFG_ADDR_NUM(spi->addrcycle)
        | OP_CFG_DUMMY_NUM(spi->erase->dummy);
    reg_write(host, reg, FMC_OP_CFG);

    reg = FMC_OP_CMD1_EN(ENABLE)
        | FMC_OP_ADDR_EN(ENABLE)
        | FMC_OP_REG_OP_START;
    reg_write(host, reg, FMC_OP);

    FMC_CMD_WAIT_CPU_FINISH(host);

    return 0;
}

/*---------------------------------------------------------------------------*/
/* hifmc100_erase */
/*---------------------------------------------------------------------------*/
static int hifmc100_erase(struct spinor_info *spinor, uint32_t addr,
        uint32_t len)
{
    struct spinor_host *host = spinor->priv;
    get_host(host);
    while (len) {
        MTD_PR(ER_DBG, "Start erase one block, addr=[0x%08x].\n",addr);
        if (hifmc100_erase_one_block(host, addr)) {
            return -EIO;
        }
        addr += spinor->dev.blocksize;
        len -= spinor->dev.blocksize;
    }
    put_host(host);
    return 0;
}

/*---------------------------------------------------------------------------*/
/* hifmc100_spi_op_map */
/*---------------------------------------------------------------------------*/
void hifmc100_spi_op_map(struct spi *spi)
{
    int ix;
    const int iftype_read[] = {
        SPI_IF_READ_STD,        SPI_IF_TYPE_STD,
        SPI_IF_READ_FAST,       SPI_IF_TYPE_STD,
        SPI_IF_READ_DUAL,       SPI_IF_TYPE_DUAL,
        SPI_IF_READ_DUAL_ADDR,  SPI_IF_TYPE_DIO,
        SPI_IF_READ_QUAD,       SPI_IF_TYPE_QUAD,
        SPI_IF_READ_QUAD_ADDR,  SPI_IF_TYPE_QIO,
        0,            0,
    };
    const int iftype_write[] = {
        SPI_IF_WRITE_STD,       SPI_IF_TYPE_STD,
        SPI_IF_WRITE_DUAL,      SPI_IF_TYPE_DUAL,
        SPI_IF_WRITE_DUAL_ADDR, SPI_IF_TYPE_DIO,
        SPI_IF_WRITE_QUAD,      SPI_IF_TYPE_QUAD,
        SPI_IF_WRITE_QUAD_ADDR, SPI_IF_TYPE_QIO,
        0,            0,
    };

    for (ix = 0; iftype_write[ix]; ix += 2) {
        if (spi->write->iftype == iftype_write[ix]) {
            spi->write->iftype = iftype_write[ix + 1];
            break;
        }
    }

    for (ix = 0; iftype_read[ix]; ix += 2) {
        if (spi->read->iftype == iftype_read[ix]) {
            spi->read->iftype = iftype_read[ix + 1];
            break;
        }
    }

    spi->erase->iftype = SPI_IF_TYPE_STD;
}

/*---------------------------------------------------------------------------*/
/* hifmc100_ids_probe */
/*---------------------------------------------------------------------------*/
static void hifmc100_ids_probe(struct spinor_info *spinor)
{
    struct spinor_host *host = spinor->priv;
    struct spi *spi = host->spi;
    struct spi_nor_info *spiinfo = spinor->dev.priv;

    MTD_PR(INIT_DBG, "\t|*-Start match SPI operation & chip init\n");
    spi->name = spiinfo->name;
    spi->cs = spinor->cur_cs;
    spi->chipsize = spiinfo->chipsize;
    spi->erasesize = spiinfo->erasesize;
    spi->addrcycle = spiinfo->addrcycle;
    spi->driver = spiinfo->driver;
    spi->host = host;

    spi_nor_search_rw(spiinfo, spi->read,
            SPI_NOR_SUPPORT_READ,
            SPI_NOR_SUPPORT_MAX_DUMMY, READ);

    spi_nor_search_rw(spiinfo, spi->write,
            SPI_NOR_SUPPORT_WRITE,
            SPI_NOR_SUPPORT_MAX_DUMMY, WRITE);

    spi_nor_get_erase(spiinfo, spi->erase);

    hifmc100_spi_op_map(spi);
    hifmc100_get_best_clock(&spi->write->clock);
    hifmc100_get_best_clock(&spi->read->clock);
    hifmc100_get_best_clock(&spi->erase->clock);

    spi->driver->qe_enable(spi);
    /* auto check fmc_addr_mode 3 bytes or 4 bytes */
    if (GET_FMC_BOOT_MODE == SPI_NOR_ADDR_MODE_3_BYTES) {
        MTD_PR(INIT_DBG, "start up mode is 3 bytes\n");
        spi->driver->entry_4addr(spi, ENABLE);
    } else
        MTD_PR(INIT_DBG, "start up mode is 4 bytes\n");

    MTD_PR(INIT_DBG, "Spi(cs%d):", spi->cs);
    MTD_PR(INIT_DBG, "Block:%sB ", ulltostr(spi->erasesize));
    MTD_PR(INIT_DBG, "Chip:%sB ", ulltostr(spi->chipsize));
    MTD_PR(INIT_DBG, "Name:\"%s\"\n", spi->name);

    MTD_PR(INIT_DBG, "\t  *-End match SPI operation & chip init\n");
}


/*---------------------------------------------------------------------------*/
/* hifmc100_set_host_addr_mode */
/*---------------------------------------------------------------------------*/
static void hifmc100_set_host_addr_mode(struct spinor_host *host, int enable)
{
    uint32_t reg;

    reg = reg_read(host, FMC_CFG);
    if (enable)
        reg |= FMC_SPI_NOR_ADDR_MODE_MASK;
    else
        reg &= ~FMC_SPI_NOR_ADDR_MODE_MASK;

    reg_write(host, reg, FMC_CFG);
}

/*---------------------------------------------------------------------------*/
/* hifmc100_shutdown */
/*---------------------------------------------------------------------------*/
void hifmc100_shutdown(void)
{
    struct spi *spi = host->spi;
    spi->driver->entry_4addr(spi, DISABLE);
}

/*---------------------------------------------------------------------------*/
/* hifmc100_host_init */
/*---------------------------------------------------------------------------*/
static int hifmc100_host_init(struct spinor_host *host)
{
    int reg, flash_type;
    MTD_PR(INIT_DBG, "\t||*-Start SPI Nor host init\n");

    if(LOS_OK != LOS_MuxCreate(&host->lock))
        return -1;

    host->regbase = (void *)FMC_REG_BASE;
    host->membase = (void *)FMC_MEM_BASE;

    reg = reg_read(host, FMC_CFG);
    flash_type = (reg & FLASH_SEL_MASK) >> FLASH_SEL_SHIFT;
    if (flash_type != FLASH_TYPE_SPI_NOR) {
        WARN_MSG("Flash type isn't Spi Nor!\n");
        return -ENODEV;
    }

    if ((reg & OP_MODE_MASK) == OP_MODE_BOOT)
        reg |= FMC_CFG_OP_MODE(OP_MODE_NORMAL);
    reg_write(host, reg, FMC_CFG);

    host->set_system_clock = hifmc100_set_system_clock;
    host->set_host_addr_mode = hifmc100_set_host_addr_mode;

    host->buffer = memalign(CACHE_ALIGNED_SIZE,
            ALIGN(CACHE_ALIGNED_SIZE, CACHE_ALIGNED_SIZE));
    if (host->buffer == NULL) {
        ERR_MSG("no mem for hifmc dma mem!\n");
        free(host);
        return -ENOMEM;
    }
    host->dma_buffer = (uint32_t )host->buffer;

    host->set_system_clock(0, ENABLE);

    reg = TIMING_CFG_TCSH(CS_HOLD_TIME)
        | TIMING_CFG_TCSS(CS_SETUP_TIME)
        | TIMING_CFG_TSHSL(CS_DESELECT_TIME);
    reg_write(host, reg, FMC_SPI_TIMING_CFG);

    reg = ALL_BURST_ENABLE;
    reg_write(host, reg, FMC_DMA_AHB_CTRL);

    MTD_PR(INIT_DBG, "\t||*-End SPI Nor host init\n");
#ifdef LOSCFG_SHELL
    (void)osReHookFuncAdd((STORAGE_HOOK_FUNC)hifmc100_shutdown, NULL);
#endif
    return 0;
}

/*---------------------------------------------------------------------------*/
/* hifmc100_resume - resume host */
/*---------------------------------------------------------------------------*/
static int hifmc100_resume(struct spinor_info *spinor)
{
    struct spinor_host *host = spinor->priv;

	put_host(host);
	MTD_PR(INIT_DBG, "\t hifmc100_resume ok\n");
	return 0;
}

/*---------------------------------------------------------------------------*/
/* hifmc100_spinor_init - spinor info member init */
/*---------------------------------------------------------------------------*/
static void hifmc100_spinor_init(struct spinor_info *spinor)
{
    spinor->erase = hifmc100_erase;
    spinor->write = hifmc100_write;
    spinor->read = hifmc100_read;

    spinor->read_id = hifmc100_read_id;
    spinor->ids_probe = hifmc100_ids_probe;
    spinor->resume = hifmc100_resume;

    spinor->cur_cs = HIFMC100_SPI_NOR_CS_NUM;
}

/*---------------------------------------------------------------------------*/
/* spinor_host_init - spinor controller initializtion entry */
/*---------------------------------------------------------------------------*/
int spinor_host_init(struct spinor_info *spinor)
{
    host = zalloc(sizeof(struct spinor_host));
    if(host == NULL) {
        ERR_MSG("no mem for spinor_host!\n");
        return -1;
    }

    spinor->priv = host;
    hifmc100_spinor_init(spinor);

    host->spinor = spinor;
    if(hifmc100_host_init(host)) {
        free(host);
        ERR_MSG("hifmc100 host init fail!\n");
        return -1;
    }

    return 0;
}

