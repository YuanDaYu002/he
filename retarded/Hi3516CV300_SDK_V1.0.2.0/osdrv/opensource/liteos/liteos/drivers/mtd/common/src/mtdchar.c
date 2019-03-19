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

#if defined(LOSCFG_FS_YAFFS) || defined(LOSCFG_FS_JFFS)

#include "fs/fs.h"
#include "linux/mtd/mtd.h"
#include "stdio.h"
#include "string.h"
#include "asm/errno.h"

#include "los_mux.h"

#include "mtd_partition.h"

/*
 * open device interface
 */
static int mtdchar_open(FAR struct file *filep)
{
    struct inode * inode_p = filep -> f_inode ;
    mtd_partition *partition = (mtd_partition *)(inode_p->i_private);

    if(partition->user_num != 0)// be opened
        return -EBUSY;

    struct mtd_info *mtd = (struct mtd_info*)(partition->mtd_info);
    size_t block_size = mtd->erasesize;

    (void)LOS_MuxPend(partition->lock, LOS_WAIT_FOREVER);

    partition->user_num = 1;
    filep->f_pos = partition->start_block * block_size;

    (void)LOS_MuxPost(partition->lock);

    return ENOERR;
}

/*
 * close device interface
 */
static int mtdchar_close(FAR struct file *filep)
{
    struct inode * inode_p = filep -> f_inode ;
    mtd_partition *partition = (mtd_partition *)(inode_p->i_private);

    (void)LOS_MuxPend(partition->lock, LOS_WAIT_FOREVER);

    partition->user_num = 0;

    (void)LOS_MuxPost(partition->lock);

    return ENOERR;
}

/*
 * read device interface
 */
static ssize_t mtdchar_read(FAR struct file *filep, FAR char *buffer, size_t buflen)
{
    struct inode * inode_p = filep -> f_inode ;
    mtd_partition *partition = (mtd_partition *)(inode_p->i_private);

    (void)LOS_MuxPend(partition->lock, LOS_WAIT_FOREVER);

    struct mtd_info *mtd = (struct mtd_info*)(partition->mtd_info);
    size_t block_size = mtd->erasesize;
    size_t start_addr = partition->start_block * block_size;
    size_t end_addr = (partition->end_block + 1) * block_size;

    size_t retlen;
    ssize_t ret = 0;
    off_t ppos = filep->f_pos;

    if(ppos > end_addr || ppos < start_addr) {
        ret = -1;
        goto out1;
    }

    if (ppos + buflen > end_addr) {
        ret = -1;
        goto out1;
    }

    if (!buflen) {
        ret = 0;
        goto out1;
    }

    ret = mtd->read(mtd, ppos, buflen, &retlen, buffer);

    if(ret) {
        goto out1;
    }

    filep->f_pos += retlen;

    ret = (ssize_t)retlen;

out1:
    (void)LOS_MuxPost(partition->lock);
    return ret;
}

/*
 * write device interface
 */
static ssize_t mtdchar_write(FAR struct file *filep, FAR const char *buffer, size_t buflen)
{
    struct inode * inode_p = filep -> f_inode ;
    mtd_partition *partition = (mtd_partition *)(inode_p->i_private);

    (void)LOS_MuxPend(partition->lock, LOS_WAIT_FOREVER);

    struct mtd_info *mtd = (struct mtd_info*)(partition->mtd_info);
    size_t block_size = mtd->erasesize;
    size_t start_addr = partition->start_block * block_size;
    size_t end_addr = (partition->end_block + 1) * block_size;
    size_t retlen;
    int ret=0;
    off_t ppos = filep->f_pos;

    if(ppos > end_addr || ppos < start_addr) {
        ret = -1;
        goto out1;
    }

    if (ppos + buflen > end_addr) {
        ret = -1;
        goto out1;
    }

    if (!buflen) {
        ret = 0;
        goto out1;
    }

    ret = mtd->write(mtd, ppos, buflen, &retlen, buffer);

    if(ret) {
        goto out1;
    }

    filep->f_pos += retlen;

    ret = (ssize_t)retlen;

out1:
    (void)LOS_MuxPost(partition->lock);
    return ret;
}

/*
 * lseek device interface
 */
static off_t mtdchar_lseek(FAR struct file *filep, off_t offset, int whence)
{
    struct inode * inode_p = filep -> f_inode ;
    mtd_partition *partition = (mtd_partition *)(inode_p->i_private);

    (void)LOS_MuxPend(partition->lock, LOS_WAIT_FOREVER);

    struct mtd_info *mtd = (struct mtd_info*)(partition->mtd_info);
    size_t block_size = mtd->erasesize;
    size_t end_addr = (partition->end_block + 1) * block_size;
    size_t start_addr = partition->start_block * block_size;

    switch (whence) {
        case SEEK_SET:
            if (offset >= 0 && (size_t)offset < end_addr - start_addr) {
                filep->f_pos = start_addr + offset;
                goto out1;
            } else
                goto err1;

        case SEEK_CUR:
            if(offset + (size_t)filep->f_pos >= start_addr &&
                    (size_t)(offset + filep->f_pos) < end_addr) {
                filep->f_pos += offset;
                goto out1;
            } else
                goto err1;

        case SEEK_END:
            if(offset < 0 && offset + end_addr >= start_addr) {
                filep->f_pos = (off_t)(offset + end_addr);
                goto out1;
            } else
                goto err1;

        default:
                goto err1;
    }
err1:
    (void)LOS_MuxPost(partition->lock);
    return -EINVAL;
out1:
    (void)LOS_MuxPost(partition->lock);
    return filep->f_pos;
}

/*
 * ioctl device interface
 */
static int mtdchar_ioctl(FAR struct file *filep, int cmd, unsigned long arg)
{
    int ret = ENOERR;
    struct inode * inode_p = filep -> f_inode ;
    mtd_partition *partition = (mtd_partition *)(inode_p->i_private);

    (void)LOS_MuxPend(partition->lock, LOS_WAIT_FOREVER);

    struct mtd_info *mtd = (struct mtd_info*)(partition->mtd_info);
    size_t block_size = mtd->erasesize;
    size_t start_addr = partition->start_block * block_size;
    size_t end_addr = (partition->end_block + 1) * block_size;
    switch (cmd) {

        case MEMGETINFO:
            {
                memcpy((void *)arg, (void *)mtd, (size_t)sizeof(struct mtd_info));
                ((struct mtd_info *)arg)->size = end_addr - start_addr;
                break;
            }

        case MEMERASE:
        case MEMERASE64:
            {
                struct erase_info *erase = (struct erase_info *)arg;
                mtd->erase(mtd, erase);
                ret = (erase->state == MTD_ERASE_FAILED)?-EIO:0;
                break;
            }
        case MEMGETBADBLOCK:
            {
                loff_t offs = *((loff_t *)arg);
                if (mtd->block_isbad != NULL)
                        ret = mtd->block_isbad(mtd, offs);
                else
                        ret = -EPERM;
                break;
            }

        case MEMSETBADBLOCK:
            {
                loff_t offs = *((loff_t *)arg);
                if (mtd->block_markbad != NULL)
                        ret = mtd->block_markbad(mtd, offs);
                else
                        ret = -EPERM;
                break;
            }

        default:
            ret = -EINVAL;
    }

    (void)LOS_MuxPost(partition->lock);

    return ret;
}

const struct file_operations_vfs g_mtdchar_fops =
{
    .open   =   mtdchar_open,
    .close  =   mtdchar_close,
    .read   =   mtdchar_read,
    .write  =   mtdchar_write,
    .seek   =   mtdchar_lseek,
    .ioctl  =   mtdchar_ioctl,
#ifndef CONFIG_DISABLE_POLL
    .poll   =   NULL,
#endif
    .unlink =   NULL
};

#endif
