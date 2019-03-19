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

#include "hisoc/spinor.h"

#include "host_common.h"
#include "spinor_common.h"
#include "spi_common.h"
#include "hisfc350.h"

static struct spinor_host *host;

/*---------------------------------------------------------------------------*/
/* hisfc350_read_id */
/*---------------------------------------------------------------------------*/
static void hisfc350_read_id(struct spinor_info *spinor, char *id)
{
    int reg, regindex = 0;
    int numread = SPI_NOR_MAX_ID_LEN;
    unsigned int *ptr = (unsigned int *)id;
    struct spinor_host *host = spinor->priv;

    reg = SPI_CMD_RDID;
    reg_write(host, reg, HISFC350_CMD_INS);

    reg = HISFC350_CMD_CONFIG_SEL_CS(spinor->cur_cs)
        | HISFC350_CMD_CONFIG_RW_READ
        | HISFC350_CMD_CONFIG_DATA_EN
        | HISFC350_CMD_CONFIG_DATA_CNT(numread)
        | HISFC350_CMD_CONFIG_START;
    reg_write(host, reg, HISFC350_CMD_CONFIG);

    HISFC350_CMD_WAIT_CPU_FINISH(host);

    while (numread) {
        *ptr = reg_read(host,
            HISFC350_CMD_DATABUF0 + regindex);
        ptr      += 1;
        regindex += 4;
        numread  -= 4;
    }
}

/*---------------------------------------------------------------------------*/
/* hisfc350_dma_transfer: dma read/write operations */
/*---------------------------------------------------------------------------*/
static void hisfc350_dma_transfer(struct spinor_host *host,
   uint32_t spi_start_addr, char *dma_buffer,
    uint8_t rw,uint32_t size)
{
    struct spi *spi = host->spi;

    if (rw)
        mtd_dma_cache_clean(dma_buffer, size);
    else
        mtd_dma_cache_inv(dma_buffer, size);
    reg_write(host, dma_buffer, HISFC350_BUS_DMA_MEM_SADDR);

    reg_write(host, spi_start_addr, HISFC350_BUS_DMA_FLASH_SADDR);

    reg_write(host, HISFC350_BUS_DMA_LEN_DATA_CNT(size), HISFC350_BUS_DMA_LEN);


    reg_write(host,
        HISFC350_BUS_DMA_CTRL_RW(rw?0:1)
        | HISFC350_BUS_DMA_CTRL_CS(spi->cs)
        | HISFC350_BUS_DMA_CTRL_START, HISFC350_BUS_DMA_CTRL);

    HISFC350_DMA_WAIT_CPU_FINISH(host);
}

/*---------------------------------------------------------------------------*/
/* hisfc350_dma_read */
/*---------------------------------------------------------------------------*/
static int hisfc350_dma_read(struct spinor_host *host, uint32_t from,
        uint32_t len, const char *buf)
{
    int num, reg;
    unsigned char *ptr = (unsigned char *)buf;
    struct spi *spi = host->spi;

    reg = spi->driver->wait_ready(spi);
    if (reg) {
        ERR_MSG("Dma read wait ready fail! reg:%#x\n", reg);
        return -EBUSY;
    }
    spi->driver->bus_prepare(spi, READ);

    host->set_system_clock(spi->read->clock, ENABLE);

    if (from & HISFC350_DMA_ALIGN_MASK) {
        num = HISFC350_DMA_ALIGN_SIZE -
            (from & HISFC350_DMA_ALIGN_MASK);
        if (num > len)
            num = len;
        hisfc350_dma_transfer(host, from,
            (unsigned char *)host->dma_buffer, READ,num);
        memcpy(ptr, host->buffer, num);
        from  += num;
        ptr += num;
        len -= num;
    }

    while (len > 0) {
        num = len;
        while (num >= HISFC350_DMA_MAX_SIZE) {
            hisfc350_dma_transfer(host, from,
                (unsigned char *)host->dma_buffer, READ,
                HISFC350_DMA_MAX_SIZE);
            memcpy(ptr, host->buffer, HISFC350_DMA_MAX_SIZE);
            ptr  += HISFC350_DMA_MAX_SIZE;
            from += HISFC350_DMA_MAX_SIZE;
            len  -= HISFC350_DMA_MAX_SIZE;
            num  -= HISFC350_DMA_MAX_SIZE;
        }

        if (num) {
            hisfc350_dma_transfer(host, from,
                (unsigned char *)host->dma_buffer, READ, num);
            memcpy(ptr, host->buffer, num);
            from += num;
            ptr  += num;
            len  -= num;
        }
    }

    return 0;
}

/*---------------------------------------------------------------------------*/
/* hisfc350_read */
/*---------------------------------------------------------------------------*/
static int hisfc350_read(struct spinor_info *spinor, uint32_t from,
        uint32_t len, const char *buf)
{
    int result;
    struct spinor_host *host = spinor->priv;

    MTD_PR(ER_DBG, "Start read buf=0x%08x from=0x%08x len=0x%08x.\n",
            buf, from, len);
    get_host(host);
    result = hisfc350_dma_read(host, from, len, buf);
    put_host(host);

    return result;
}

/*---------------------------------------------------------------------------*/
/* hisfc350_dma_write */
/*---------------------------------------------------------------------------*/
static int hisfc350_dma_write(struct spinor_host *host, uint32_t to,
        uint32_t len, const char *buf)
{
    int num, reg;
    unsigned char *ptr = (unsigned char *)buf;
    struct spi *spi = host->spi;

    reg = spi->driver->wait_ready(spi);
    if (reg) {
        ERR_MSG("Dma write wait ready fail! reg:%#x\n", reg);
        return -EBUSY;
    }
    spi->driver->write_enable(spi);
    spi->driver->bus_prepare(spi, WRITE);

    host->set_system_clock(spi->write->clock, ENABLE);

    if (to & HISFC350_DMA_ALIGN_MASK) {
        num = HISFC350_DMA_ALIGN_SIZE - (to & HISFC350_DMA_ALIGN_MASK);
        if (num > len)
            num = len;
        memcpy(host->buffer, ptr, num);
        hisfc350_dma_transfer(host, to,
            (unsigned char *)host->dma_buffer, WRITE, num);

        to  += num;
        ptr += num;
        len -= num;
    }

    while (len > 0) {
        num = ((len >= HISFC350_DMA_MAX_SIZE) ? HISFC350_DMA_MAX_SIZE : len);

        memcpy(host->buffer, ptr, num);
        hisfc350_dma_transfer(host, to,
            (unsigned char *)host->dma_buffer, WRITE, num);

        to  += num;
        ptr += num;
        len -= num;
    }

    return 0;
}

/*---------------------------------------------------------------------------*/
/* hisfc350_write */
/*---------------------------------------------------------------------------*/
static int hisfc350_write(struct spinor_info *spinor, uint32_t to, uint32_t len,
                    const char *buf)
{
    int result;
    struct spinor_host *host = spinor->priv;

    MTD_PR(ER_DBG, "Start write buf=0x%08x to=0x%08x len=0x%08x.\n", buf, to, len);
    get_host(host);
    result = hisfc350_dma_write(host, to, len, buf);
    put_host(host);

    return result;
}

/*---------------------------------------------------------------------------*/
/* hisfc350_erase_one_block */
/*---------------------------------------------------------------------------*/
static int hisfc350_erase_one_block(struct spinor_host *host, uint32_t offset)
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

    reg = spi->erase->cmd;
    reg_write(host, reg, HISFC350_CMD_INS);

    reg = offset & HISFC350_CMD_ADDR_MASK;
    reg_write(host, reg, HISFC350_CMD_ADDR);

    reg = HISFC350_CMD_CONFIG_SEL_CS(spi->cs)
        | HISFC350_CMD_CONFIG_MEM_IF_TYPE(spi->erase->iftype)
        | HISFC350_CMD_CONFIG_DUMMY_CNT(spi->erase->dummy)
        | HISFC350_CMD_CONFIG_ADDR_EN
        | HISFC350_CMD_CONFIG_START;
    reg_write(host, reg, HISFC350_CMD_CONFIG);

    HISFC350_CMD_WAIT_CPU_FINISH(host);

    return 0;
}

/*---------------------------------------------------------------------------*/
/* hisfc350_erase */
/*---------------------------------------------------------------------------*/
static int hisfc350_erase(struct spinor_info *spinor, uint32_t addr,
        uint32_t len)
{
    struct spinor_host *host = spinor->priv;

    get_host(host);
    while (len) {
        MTD_PR(ER_DBG, "Start erase one block, addr=[0x%08x].\n",addr);
        if (hisfc350_erase_one_block(host, addr)) {
            return -EIO;
        }
        addr += spinor->dev.blocksize;
        len -= spinor->dev.blocksize;
    }
    put_host(host);
    return 0;
}

/*---------------------------------------------------------------------------*/
/* hisfc350_map_chipsize */
/*---------------------------------------------------------------------------*/
static int hisfc350_map_chipsize(unsigned long long chipsize)
{
    int shift = 0;
    chipsize >>= (19 - 3); /* 19: 512K; 3: Bytes -> bit */

    while (chipsize) {
        chipsize >>= 1;
        shift++;
    }
    return shift;
}

/*---------------------------------------------------------------------------*/
/* hisfc350spi_op_map */
/*---------------------------------------------------------------------------*/
void hisfc350_spi_op_map(struct spi *spi) {
    int ix;
    const int iftype_read[] = {
        SPI_IF_READ_STD,       HISFC350_SPI_IF_TYPE_STD,
        SPI_IF_READ_FAST,      HISFC350_SPI_IF_TYPE_STD,
        SPI_IF_READ_DUAL,      HISFC350_SPI_IF_TYPE_DUAL,
        SPI_IF_READ_DUAL_ADDR, HISFC350_SPI_IF_TYPE_DIO,
        SPI_IF_READ_QUAD,      HISFC350_SPI_IF_TYPE_QUAD,
        SPI_IF_READ_QUAD_ADDR, HISFC350_SPI_IF_TYPE_QIO,
        0, 0,
    };
    const int iftype_write[] = {
        SPI_IF_WRITE_STD,       HISFC350_SPI_IF_TYPE_STD,
        SPI_IF_WRITE_DUAL,      HISFC350_SPI_IF_TYPE_DUAL,
        SPI_IF_WRITE_DUAL_ADDR, HISFC350_SPI_IF_TYPE_DIO,
        SPI_IF_WRITE_QUAD,      HISFC350_SPI_IF_TYPE_QUAD,
        SPI_IF_WRITE_QUAD_ADDR, HISFC350_SPI_IF_TYPE_QIO,
        0, 0,
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
/* hisfc350_ids_probe */
/*---------------------------------------------------------------------------*/
static void hisfc350_ids_probe(struct spinor_info *spinor)
{
    struct spinor_host *spinor_host = spinor->priv;
    struct spi *spi = spinor_host->spi;
    struct spi_nor_info *spiinfo = spinor->dev.priv;
    int reg;

    MTD_PR(INIT_DBG, "\t|*-Start match SPI operation & chip init\n");
    spi->name = spiinfo->name;
    spi->cs = spinor->cur_cs;
    spi->chipsize = spiinfo->chipsize;
    spi->erasesize = spiinfo->erasesize;
    spi->addrcycle = spiinfo->addrcycle;
    spi->driver = spiinfo->driver;
    spi->host = spinor_host;

    spi_nor_search_rw(spiinfo, spi->read,
            SPI_NOR_SUPPORT_READ,
            SPI_NOR_SUPPORT_MAX_DUMMY, READ);

    spi_nor_search_rw(spiinfo, spi->write,
            SPI_NOR_SUPPORT_WRITE,
            SPI_NOR_SUPPORT_MAX_DUMMY, WRITE);

    spi_nor_get_erase(spiinfo, spi->erase);

    hisfc350_spi_op_map(spi);
    hisfc350_get_best_clock(&spi->write->clock);
    hisfc350_get_best_clock(&spi->read->clock);
    hisfc350_get_best_clock(&spi->erase->clock);

    spi->driver->qe_enable(spi);

    reg = reg_read(host, HISFC350_BUS_FLASH_SIZE);
    reg &= ~(HISFC350_BUS_FLASH_SIZE_CS0_MASK
            << (spi->cs << 3));
    reg |= (hisfc350_map_chipsize(spi->chipsize)
            << (spi->cs << 3));
    reg_write(host, reg, HISFC350_BUS_FLASH_SIZE);

    reg = SFC_MEM_BASE;
    reg_write(host, reg, (HISFC350_BUS_BASE_ADDR_CS0
             + (spi->cs << 2)));

    /* auto check sfc_addr_mode 3 bytes or 4 bytes */
    if (GET_SFC_ADDR_MODE== THREE_BYTE_ADDR_BOOT) {
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
/* hisfc350_set_host_addr_mode */
/*---------------------------------------------------------------------------*/
static void hisfc350_set_host_addr_mode(struct spinor_host *host, int enable)
{
    int reg;
    reg = reg_read(host, HISFC350_GLOBAL_CONFIG);
    if (enable)
        reg |= HISFC350_GLOBAL_CONFIG_ADDR_MODE_4B;
    else
        reg &= ~HISFC350_GLOBAL_CONFIG_ADDR_MODE_4B;

    reg_write(host, reg, HISFC350_GLOBAL_CONFIG);
}

/*---------------------------------------------------------------------------*/
/* hisfc350_host_init init hisfc350 host */
/*---------------------------------------------------------------------------*/
static int hisfc350_host_init(struct spinor_host *host)
{
    int reg;
    MTD_PR(INIT_DBG, "\t||*-Start SPI Nor host init\n");

    if(LOS_OK != LOS_MuxCreate(&host->lock))
        return -1;

    host->regbase = (void *)(SFC_REG_BASE);
    host->membase = (void *)(SFC_MEM_BASE);

    host->set_system_clock = hisfc350_set_system_clock;
    host->set_host_addr_mode = hisfc350_set_host_addr_mode;

    host->buffer = memalign(CACHE_ALIGNED_SIZE,
        ALIGN(HISFC350_DMA_MAX_SIZE, CACHE_ALIGNED_SIZE));
    if (host->buffer == NULL) {
        MTD_PR(INIT_DBG, "no mem for hisfc350 dma mem!\n");
        free(host);
        return -ENOMEM;
    }
    host->dma_buffer = (uint32_t )host->buffer;

    host->set_system_clock(0, ENABLE);

    reg = HISFC350_TIMING_TCSS(0x6)
            | HISFC350_TIMING_TCSH(0x6)
            | HISFC350_TIMING_TSHSL(0xf);
    reg_write(host, reg, HISFC350_TIMING);

    reg = HISFC350_BUS_DMA_AHB_CTRL_INCR4_EN
            | HISFC350_BUS_DMA_AHB_CTRL_INCR8_EN
            | HISFC350_BUS_DMA_AHB_CTRL_INCR16_EN;
    reg_write(host, reg, HISFC350_BUS_DMA_AHB_CTRL);

    MTD_PR(INIT_DBG, "\t||*-End SPI Nor host init\n");
    return 0;
}

/*---------------------------------------------------------------------------*/
/* hisfc350_resume - resume host */
/*---------------------------------------------------------------------------*/
static int hisfc350_resume(struct spinor_info *spinor)
{
    struct spinor_host *host = spinor->priv;

	put_host(host);
	MTD_PR(INIT_DBG, "\t hifmc100_resume ok\n");
	return 0;
}

/*---------------------------------------------------------------------------*/
/* hisfc350_spinor_init - spinor info member init */
/*---------------------------------------------------------------------------*/
static void hisfc350_spinor_init(struct spinor_info *spinor)
{
    spinor->erase = hisfc350_erase;
    spinor->write = hisfc350_write;
    spinor->read = hisfc350_read;

    spinor->read_id = hisfc350_read_id;
    spinor->ids_probe = hisfc350_ids_probe;
    spinor->resume = hisfc350_resume;

    spinor->cur_cs = HISFC350_SPI_NOR_CS_NUM;
}

/*---------------------------------------------------------------------------*/
/* spinor_host_init - spinor controller initializtion entry */
/*---------------------------------------------------------------------------*/
int spinor_host_init(struct spinor_info *spinor)
{
    host = zalloc(sizeof(struct spinor_host));
    if(host == NULL) {
        ERR_MSG("no mem for spinor_host!");
        return -1;
    }

    spinor->priv = host;
    hisfc350_spinor_init(spinor);

    host->spinor = spinor;
    if(hisfc350_host_init(host)) {
        free(host);
        ERR_MSG("hisfc350 host init fail!");
        return -1;
    }

    return 0;
}

