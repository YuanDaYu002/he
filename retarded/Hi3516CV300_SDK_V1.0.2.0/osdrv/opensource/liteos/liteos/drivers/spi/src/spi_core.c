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
#include "los_mux.h"
#include "fs/fs.h"
#include "hisoc/spi.h"
#include "spi.h"

struct spi_platform_driver_data
{
    struct spi_driver_data driver_data;
    struct spi_platform_data platform_data;
    int enable;
};

SPI_DRIVER_DATA_DECLARE(TOTAL_SPI_NUM);

/*
 * spi_set_cs: confit spi cs
 */
static int spi_set_cs(struct spi_driver_data *sdd,
        unsigned char csn, unsigned char flag)
{
    struct spi_platform_data *spd = sdd->spd;
    if (spd->cfg_cs(sdd->bus_num, csn))
        return -1;

    if (flag == SPI_CS_ACTIVE)
        sdd->enable(sdd);
    else
        sdd->disable(sdd);

    return 0;
}

/*
 * spi_txrx: send and receive data interface
 */
static int spi_txrx(struct spi_driver_data *sdd,
        struct spi_ioc_transfer *transfer)
{
    int ret = 0;
    if(transfer->speed)
        sdd->cur_speed = transfer->speed;
    else
        /* default speed is 2MHz */
        sdd->cur_speed = 2000000;

    sdd->config(sdd);
    spi_set_cs(sdd, sdd->cur_cs, SPI_CS_ACTIVE);

    ret = sdd->flush_fifo(sdd);
    if (ret)
        return ret;

    if (sdd->cur_bits_per_word <= 8)
        ret = sdd->txrx8(sdd, transfer);
    else
        ret = sdd->txrx16(sdd, transfer);

    if (ret || transfer->cs_change) {
        spi_set_cs(sdd, sdd->cur_cs, SPI_CS_INACTIVE);
    }

    return ret;
}

/*
 * spi_init_cfg: spi init configuration
 */
static int spi_init_cfg(struct spi_driver_data *sdd)
{
    struct spi_platform_data *spd = sdd->spd;

    if (spd->hw_init_cfg(sdd->bus_num))
        return -1;

    if (sdd->config(sdd))
        return -1;
    
    return 0;
}

/*
 * spi_set_platdata: set platform data for driver
 */
static int spi_set_platdata(struct spi_platform_data *spd, int id)
{

    if (spd == NULL) {
        spi_err("%s spd == NULL\n", __func__);
        return -1;
    }
    spd->clk_rate = get_bus_clk() / 4;
    spd->cfg_cs = spi_cfg_cs;
    spd->hw_init_cfg = spi_hw_init_cfg;
    spd->hw_exit_cfg = spi_hw_exit_cfg;

    return 0;
}

/*
 * spi_host_init interface: init spi host
 */
static void spi_host_init(void)
{
    int i;
    struct spi_platform_data *spd;
    struct spi_driver_data *sdd;
    for(i = 0; i < ARRAY_SIZE(pspi_pdds); i++)
    {
        if(pspi_pdds[i].enable == 0) continue;

        spi_msg("init spi master[%0d]...\n", i);
        spd = &(pspi_pdds[i].platform_data);
        sdd = &(pspi_pdds[i].driver_data);

        extern int spi_intf_init(struct spi_driver_data *sdd);
        spi_intf_init(sdd);

        if (spi_set_platdata(spd, i))
            spi_err("set platform data fail for host[%0d]!!!", i);

        sdd->spd = spd;
        sdd->max_speed = (spd->clk_rate) / ((SCR_MIN + 1) * CPSDVSR_MIN);
        sdd->min_speed = (spd->clk_rate) / ((SCR_MAX + 1) * CPSDVSR_MAX);
        
        /*
         *set default speed as 2MHz
         */
        sdd->cur_speed = 2000000;
        sdd->cur_mode = SPI_MODE_3 | SPI_LSB_FIRST;
        sdd->cur_bits_per_word = 8;

        if (spi_init_cfg(sdd))    /* Setup Deufult Mode */
            spi_err("spi init fail for host[%0d]!!!", i);
    }
}

/*
 * spi_transfer interface
 */
size_t spi_transfer(struct spi_driver_data *sdd, struct spi_ioc_transfer *wt)
{
    if(spi_txrx(sdd, wt) == 0x0)
        return wt->len;
    else
        return 0;
}

static struct spi_dev spidev[TOTAL_SPI_CS_NUM];
#ifdef LOSCFG_FS_VFS
extern struct file_operations_vfs spidev_ops;
#endif
/*
 * spi_dev_init :spi device init entry
 */
int spi_dev_init(void)
{
    int ret = 0;
    int devcnt=0;
    int i,j;
    int num_cs;
    char dev_path[50];
    
    spi_host_init();

    for(i = 0;i<TOTAL_SPI_NUM;i++)
    {
        num_cs = pspi_pdds[i].platform_data.num_cs;
        if(pspi_pdds[i].enable == 0) {
            devcnt += num_cs;
            continue;
        }
        if(num_cs>0)
        {
            for(j=0;j<num_cs;j++)
            {
                spidev[devcnt].cs_index = j;
                spidev[devcnt].cur_sdd = &(pspi_pdds[i].driver_data);
#ifdef LOSCFG_FS_VFS
                snprintf(dev_path, sizeof(dev_path), "/dev/spidev%u.%u", i, j);
                ret =  register_driver(dev_path, &spidev_ops, 0755, &spidev[devcnt]);
                if(ret)
                    spi_err("gen %s fail!\n",dev_path);
                if(j==0)
                {
                    ret = LOS_MuxCreate((UINT32 *)(&spidev[devcnt].cur_sdd->lock));
                    if(ret)
                        spi_err("create lock for spi host[%d] fail!\n",i);
                }
#endif
                devcnt++;
            }
        }
    }

    return 0;
}

/*
 * spi_set: set spi device interface
 */
int spi_dev_set(int host_no, int cs_index, struct spi_ioc_transfer * transfer)
{
    int ret;
    int num_cs;
    struct spi_driver_data *sdd = NULL;

    if(host_no<TOTAL_SPI_NUM)
    {
        num_cs = pspi_pdds[host_no].platform_data.num_cs;
        if(num_cs>0)
        {
            sdd = &(pspi_pdds[host_no].driver_data);
        }
    }

    if(sdd != NULL)
    {
        sdd->cur_cs = cs_index;
        ret = spi_transfer(sdd, transfer);
        return ret;
    }
    else
    {
        return -1;
    }
}

