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

#include <errno.h>
#include <linux/kernel.h>
#include "asm/delay.h"
#include "stdio.h"

#include "spi.h"
#include "spi-pl022.h"

#ifdef LOSCFG_DRIVERS_HIDMAC
#include "dmac_ext.h"
#endif

/*
 * spi_enable: enable spi
 */
static void spi_enable(struct spi_driver_data *sdd)
{
    unsigned int value;

    value = spi_readl(sdd->regbase + REG_SPI_CR1);

    value |= SPI_CR1_SSE;
    spi_writel(value, sdd->regbase + REG_SPI_CR1);
}

/*
 * spi_disable: disable spi
 */
static void spi_disable(struct spi_driver_data *sdd)
{
    unsigned int value;

    value = spi_readl(sdd->regbase + REG_SPI_CR1);

    value &= ~SPI_CR1_SSE;
    spi_writel(value, sdd->regbase + REG_SPI_CR1);
}

/*
 * spi_config: spi configuration
 */
static int spi_config(struct spi_driver_data *sdd)
{
    struct spi_platform_data *spd = sdd->spd;
    unsigned int tmp;
    unsigned int value;
    unsigned int scr, cpsdvsr;

    spi_disable(sdd);

    /* Check if we can provide the requested rate */
    if (sdd->cur_speed > sdd->max_speed)  /* Max possible */
        sdd->cur_speed = sdd->max_speed;

    /* Min possible */
    if (sdd->cur_speed < sdd->min_speed) {
        spi_err("config: sdd->cur_speed is %d not supported!\n",
                sdd->cur_speed);
        return -EINVAL;
    }

    /* Check if we can provide the requested bits_per_word */
    if ((sdd->cur_bits_per_word < 4) || (sdd->cur_bits_per_word > 16)) {
        spi_err("config: sdd->cur_bits_per_word is %d not supported!\n",
                sdd->cur_bits_per_word);
        return -EINVAL;
    }

    /*compute spi speed, speed=clk/(cpsdvsr*(scr+1)) */
    tmp = (spd->clk_rate) / (sdd->cur_speed);
    if (tmp < CPSDVSR_MIN) {
        cpsdvsr = CPSDVSR_MIN;
        scr = 0;
    } else if (tmp <= CPSDVSR_MAX) {
        cpsdvsr = tmp & (~0x1);
        scr = (tmp / cpsdvsr) - 1;
    } else {
        cpsdvsr = CPSDVSR_MAX;
        scr = (tmp / cpsdvsr) - 1;
    }

    /* config SPICPSR register */
    value = spi_readl(sdd->regbase + REG_SPI_CPSR);

    value &= ~SPI_CPSR_CPSDVSR;
    value |= cpsdvsr << SPI_CPSR_CPSDVSR_SHIFT;

    spi_writel(value, sdd->regbase + REG_SPI_CPSR);

    /* config SPICR0 register */
    value = spi_readl(sdd->regbase + REG_SPI_CR0);

    value &= ~SPI_CR0_DSS;
    value |= (sdd->cur_bits_per_word - 1) << SPI_CR0_DSS_SHIFT;

    value &= ~SPI_CR0_FRF;

    value &= ~SPI_CR0_SPO;
    tmp = (!!(sdd->cur_mode & SPI_CPOL)) << SPI_CR0_SPO_SHIFT;
    value |= tmp;

    value &= ~SPI_CR0_SPH;
    tmp = (!!(sdd->cur_mode & SPI_CPHA)) << SPI_CR0_SPH_SHIFT;
    value |= tmp;

    value &= ~SPI_CR0_SCR;
    value |= (scr << SPI_CR0_SCR_SHIFT);

    spi_writel(value, sdd->regbase + REG_SPI_CR0);

    /* config SPICR1 register */
    value = spi_readl(sdd->regbase + REG_SPI_CR1);

    value &= ~SPI_CR1_LBN;
    tmp = (!!(sdd->cur_mode & SPI_LOOP)) << SPI_CR1_LBN_SHIFT;
    value |= tmp;

    value &= ~SPI_CR1_MS;

    value &= ~SPI_CR1_BIG_END;
    tmp = (!!(sdd->cur_mode & SPI_LSB_FIRST)) << SPI_CR1_BIG_END_SHIFT;
    value |= tmp;

    value &= ~SPI_CR1_ALT;
    value |= 0x1 << SPI_CR1_ALT_SHIFT;

    spi_writel(value, sdd->regbase + REG_SPI_CR1);

    return 0;
}

/*
 * wait for send fifo empty and  spi bus no busy
 */
static int spi_check_timeout(struct spi_driver_data *sdd)
{
    unsigned int value;
    unsigned int tmp = 0;

    while (1) {
        value = spi_readl(sdd->regbase + REG_SPI_SR);
        if ((value & SPI_SR_TFE) && (!(value & SPI_SR_BSY)))
            break;

        if (tmp++ > MAX_WAIT) {
            spi_err("%s spi transfer wait timeout!\n",
                    __func__);
            return -EIO;
        }

        udelay(1);
    }

    return 0;
}

/*
 * spi_flush_fifo: flush spi fifo
 */
static int spi_flush_fifo(struct spi_driver_data *sdd)
{
    unsigned int value;
    unsigned int tmp;

    tmp = spi_check_timeout(sdd);
    if (tmp)
        return tmp;

    tmp = 0;
    while (1) {
        value = spi_readl(sdd->regbase + REG_SPI_SR);
        if (!(value & SPI_SR_RNE))
            break;

        if (tmp++ > sdd->spd->fifo_size) {
            spi_err("%s spi transfer check rx fifo wait timeout!\n",
                    __func__);
            return -EIO;
        }

        spi_readl(sdd->regbase + REG_SPI_DR);
    }

    return 0;
}

/*
 * spi_txrx8: send and receive 8bit data
 */
static int spi_txrx8(struct spi_driver_data *sdd, struct spi_ioc_transfer *st)
{
    unsigned int    len = st->len;
    unsigned int    tmp_len;
    unsigned int    count;
    const uint8_t    *tx = (const uint8_t *)(st->tx_buf);
    uint8_t    *rx = (uint8_t *)(st->rx_buf);
    uint8_t     value;
    unsigned int tmp;

    while (len) {
        if (len > sdd->spd->fifo_size)
            tmp_len = sdd->spd->fifo_size;
        else
            tmp_len = len;

        len -= tmp_len;

        /* wirte fifo */
        count = tmp_len;
        value = 0;
        while (count) {
            if (tx)
                value = *tx++;

            spi_writel(value, sdd->regbase + REG_SPI_DR);
            count -= 1;
            spi_readl(sdd->regbase + REG_SPI_SR);
        }

        tmp = spi_check_timeout(sdd);
        if (tmp)
            return tmp;

        /* read fifo*/
        count = tmp_len;
        while (count) {
            value = spi_readl(sdd->regbase + REG_SPI_DR);
            if (rx)
                *rx++ = value;

            count -= 1;
            spi_readl(sdd->regbase + REG_SPI_SR);
        }

    }
    return 0;
}

/*
 * spi_txrx16: send and receive 16bit data
 */
static int spi_txrx16(struct spi_driver_data *sdd,
        struct spi_ioc_transfer *st)
{
    unsigned int    len = st->len;
    unsigned int    tmp_len = 0;
    unsigned int    count = 0;
    const uint16_t    *tx = (const uint16_t *)(st->tx_buf);
    uint16_t    *rx = (uint16_t *)(st->rx_buf);
    uint16_t        value = 0;
    unsigned int tmp;

    while (len) {
        tmp = sdd->spd->fifo_size * 2;
        if (len > tmp)
            tmp_len = tmp;
        else
            tmp_len = len;

        len -= tmp_len;

        /* wirte fifo */
        count = tmp_len;
        value = 0;
        while (count >= 2) {
            if (tx)
                value = *tx++;

            spi_writel(value, sdd->regbase + REG_SPI_DR);
            count -= 2;
        }

        tmp = spi_check_timeout(sdd);
        if (tmp)
            return tmp;

        /* read fifo*/
        count = tmp_len;
        while (count >= 2) {
            value = spi_readl(sdd->regbase + REG_SPI_DR);
            if (rx)
                *rx++ = value;

            count -= 2;
        }
    }
    return 0;
}

#ifdef LOSCFG_DRIVERS_HIDMAC
#ifndef TX
#define TX  1
#endif
#ifndef RX
#define RX  0
#endif
int spi_dma_txrx_work(struct spi_driver_data *sdd,
        struct spi_ioc_transfer *transfer)
{
    int ret = 0;
    const char *tx_buff = (const uint8_t *)(transfer->tx_buf);
    char *rx_buff = (uint8_t *)(transfer->rx_buf);
    char *tx = NULL;
    char *rx = NULL;

    int peripheralid;
    int full_duplex_dmac_rx_ch = -1;
    int full_duplex_dmac_tx_ch = -1;

    tx = (char *)memalign(CACHE_ALIGNED_SIZE, (transfer->len+ CACHE_ALIGNED_SIZE) & (~(CACHE_ALIGNED_SIZE - 1)));
    if (tx == NULL) {
        spi_err("tx allocate space fail!\n");
        ret = -1;
        goto out;
    }

    rx = (char *)memalign(CACHE_ALIGNED_SIZE, (transfer->len+ CACHE_ALIGNED_SIZE) & (~(CACHE_ALIGNED_SIZE - 1)));
    if (rx == NULL) {
        spi_err("rx allocate space fail!\n");
        ret = -1;
        goto free_tx;
    }

    memcpy(tx, tx_buff, transfer->len);

    peripheralid = dmac_check_request((unsigned int)(sdd->regbase + REG_SPI_DR), RX);
    if (peripheralid < 0) {
        spi_err("Invalid devaddr.\n");
        goto free_rx;
    }
    full_duplex_dmac_rx_ch = dmac_transfer_without_start_ch((unsigned int)rx,
            peripheralid, RX, transfer->len);

    peripheralid = dmac_check_request((unsigned int)(sdd->regbase + REG_SPI_DR), TX);
    if (peripheralid < 0) {
        spi_err("Invalid devaddr.\n");
        goto free_ch;
    }
    full_duplex_dmac_tx_ch = dmac_transfer_without_start_ch((unsigned int)tx,
            peripheralid, TX, transfer->len);

    dmac_channelstart(full_duplex_dmac_rx_ch);
    dmac_channelstart(full_duplex_dmac_tx_ch);

    full_duplex_dma_ch_wait(full_duplex_dmac_rx_ch, full_duplex_dmac_tx_ch);

    memcpy(rx_buff, (const char *)rx, transfer->len);

free_ch:
    full_duplex_dma_ch_free(full_duplex_dmac_rx_ch, full_duplex_dmac_tx_ch);
free_rx:
    free(rx);
free_tx:
    free(tx);
out:
    return ret;
}

int spi_dma_txrx(struct spi_driver_data *sdd,
        struct spi_ioc_transfer *transfer)
{
    int ret = 0;
    unsigned int value;

    /* config FIFO water line and burst, enable dma */
    spi_writel(DEF_TX_FIFO_CFG , sdd->regbase + REG_SPI_TXFIFO_CR);
    spi_writel(DEF_RX_FIFO_CFG, sdd->regbase + REG_SPI_RXFIFO_CR);
    spi_writel(DMA_ENABLE, sdd->regbase + REG_SPI_DMA_CR);

    ret = spi_dma_txrx_work(sdd, transfer);
    spi_writel(DMA_DISABLE, sdd->regbase + REG_SPI_DMA_CR);

    return ret;
}
#endif

/*
 * spi_intf_init: register spi operation interface
 */
int spi_intf_init(struct spi_driver_data *sdd)
{
    if(sdd == NULL) {
        spi_err("sdd is NULL!!!\n");
        return -1;
    }
    sdd->config = spi_config;
    sdd->enable = spi_enable;
    sdd->disable = spi_disable;
    sdd->flush_fifo = spi_flush_fifo;
#ifdef DRIVERS_SPI_DMA_MODE
    sdd->txrx8 = spi_dma_txrx;
    sdd->txrx16 = spi_dma_txrx;
#else
    sdd->txrx8 = spi_txrx8;
    sdd->txrx16 = spi_txrx16;
#endif
    return 0;
}

