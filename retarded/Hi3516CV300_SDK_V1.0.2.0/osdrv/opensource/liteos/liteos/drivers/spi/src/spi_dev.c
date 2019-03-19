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
#ifdef LOSCFG_FS_VFS
#include "fs/fs.h"
#include "los_mux.h"
#include "spi.h"
#include "spi_dev.h"
#include "errno.h"

/*
 * spidev_open: open spi devide
 */
static int spidev_open(struct file *filep)
{
    struct inode *inode = filep->f_inode ;
    int cs_index;

    cs_index = ((struct spi_dev *)(inode->i_private))->cs_index;
    ((struct spi_dev *)(inode->i_private))->cur_sdd->cur_cs = cs_index;

    return ENOERR;
}

/*
 * spidev_ioctrl: ioctrl spi device interface
 */
static int spidev_ioctl(struct file * filep, int cmd, unsigned long arg)
{
    int ret = ENOERR;
    unsigned int tmp = 0;
    struct spi_ioc_transfer * transfer;
    struct inode * inode = filep -> f_inode ;
    struct spi_driver_data *sdd =
            ((struct spi_dev *)(inode->i_private))->cur_sdd;

    if (arg == (unsigned long)NULL) {
        spi_err("arg is NULL!\n", cmd);
        return -1;
    }

    (void)LOS_MuxPend(sdd->lock, LOS_WAIT_FOREVER);
    switch(cmd) {
        /* read requests */
        case SPI_IOC_RD_MODE:
            *(unsigned int *)arg = sdd->cur_mode;
            break;
        case SPI_IOC_RD_LSB_FIRST:
            spi_err("Not support cmd(%0d)!!!\n", cmd);
            ret = -EINVAL;
            break;
        case SPI_IOC_RD_BITS_PER_WORD:
            *(unsigned int *)arg = sdd->cur_bits_per_word;
            break;
        case SPI_IOC_RD_MAX_SPEED_HZ:
            *(unsigned long *)arg = sdd->max_speed;
            break;
        /* write requests */
        case SPI_IOC_WR_MODE:
            tmp = *(unsigned int *) arg;
            sdd->cur_mode = tmp;
            break;
        case SPI_IOC_WR_LSB_FIRST:
            spi_err("Not support cmd(%0d)!!!\n", cmd);
            ret = -EINVAL;
            break;
        case SPI_IOC_WR_BITS_PER_WORD:
            tmp = *(unsigned int *) arg;
            sdd->cur_bits_per_word = tmp;
            break;
        case SPI_IOC_WR_MAX_SPEED_HZ:
            tmp = *(unsigned int *) arg;
            sdd->max_speed = tmp;
            break;
        case SPI_IOC_MESSAGE(N):
            transfer = (struct spi_ioc_transfer *) arg;
            ret = spi_transfer(sdd, transfer);
            if (ret != transfer -> len) {
                spi_err("fail to ioctl! \n");
                ret = -EIO;
            }
            break;
        default:
            spi_err("Not support cmd(%0d)!!!\n", cmd);
            ret = -EINVAL;
            break;
    }
    (void)LOS_MuxPost(sdd->lock);

    return ret;
}

struct file_operations_vfs spidev_ops =
{
    spidev_open,
    NULL,
    NULL,
    NULL,
    NULL,
    spidev_ioctl,
#ifndef CONFIG_DISABLE_POLL
    NULL,
#endif
    NULL
};

#endif /*end of LOSCFG_FS_VFS*/

