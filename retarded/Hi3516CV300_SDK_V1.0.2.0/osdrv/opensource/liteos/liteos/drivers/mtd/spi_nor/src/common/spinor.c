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

#include "errno.h"
#include "stdio.h"
#include "stdlib.h"

#include "linux/mtd/mtd.h"

#include "fs/fs.h"

#include "spinor.h"

#include "mtd_common.h"
#include "spinor_common.h"
#include "spinor_scan.h"
#include "spinor_ops.h"

/*---------------------------------------------------------------------------*/
struct mtd_info *spinor_mtd = NULL;

/*---------------------------------------------------------------------------*/
/* spinor erase, register to mtd->erase */
/*---------------------------------------------------------------------------*/
static int spinor_erase(struct mtd_info *mtd, struct erase_info *ops)
{
    struct spinor_info *spinor = (struct spinor_info *)mtd->priv;
    uint32_t offset = ops->addr;
    uint32_t length = ops->len;

    MTD_PR(ER_DBG, "addr = 0x%0x, len = 0x%0x.\n", offset, length);

    if (offset + length > mtd->size) {
        ERR_MSG("erase area is out of range!\n");
        return -EINVAL;
    }

    if (offset & (mtd->erasesize-1)) {
        ERR_MSG("erase start address is not aligned!\n");
        return -EINVAL;
    }

    if (length & (mtd->erasesize-1)) {
        ERR_MSG("erase length is not aligned!\n");
        return -EINVAL;
    }

    if(spinor->erase(spinor, offset, length)) {
        ERR_MSG("erase fail!\n");
        ops->state = MTD_ERASE_FAILED;
        ops->fail_addr = offset;
        return -EIO;
    } else {
        ops->state = MTD_ERASE_DONE;
        return 0;
    }
}

/*---------------------------------------------------------------------------*/
/* spinor write - register to mtd->write */
/*---------------------------------------------------------------------------*/
static int spinor_write(struct mtd_info *mtd, loff_t to, size_t len,
            size_t *retlen, const char *buf)
{
    struct spinor_info *spinor = (struct spinor_info *)mtd->priv;

    MTD_PR(WR_DBG, "to = 0x%0x, len = 0x%0x, buf = 0x%0x.\n",
            (uint32_t)to, (uint32_t)len, (uint32_t)buf);
    *retlen = len;
    if ((to + len) > mtd->size) {
        ERR_MSG("write area is out of range!\n");
        return -EINVAL;
    }

    if (!len) {
        DBG_MSG("Warning:write length is 0!\n");
        *retlen = 0;
        return 0;
    }

    return spinor->write(spinor, (uint32_t)to, (uint32_t)len, buf);
}

/*---------------------------------------------------------------------------*/
/* spinor read - register to mtd->read */
/*---------------------------------------------------------------------------*/
static int spinor_read(struct mtd_info *mtd, loff_t from, size_t len,
            size_t *retlen, const char *buf)
{
    struct spinor_info *spinor = (struct spinor_info *)mtd->priv;

    MTD_PR(RD_DBG, "from = 0x%0x, len = 0x%0x, buf = 0x%0x.\n",
            (uint32_t)from, (uint32_t)len, (uint32_t)buf);
    *retlen = len;
    if ((from + len) > mtd->size) {
        ERR_MSG("read area is out of range!\n");
        return -EINVAL;
    }

    if (!len) {
        DBG_MSG("Warning:read length is 0!\n");
        *retlen = 0;
        return 0;
    }

    return spinor->read(spinor, (uint32_t)from, (uint32_t)len, buf);
}

/*---------------------------------------------------------------------------*/
/* spinor_register - spinor operations func register to mtd_info struct */
/*---------------------------------------------------------------------------*/
static void spinor_register(struct mtd_info *mtd)
{
    struct spinor_info *spinor = (struct spinor_info *)mtd->priv;

    mtd->size = spinor->dev.chipsize;
    mtd->erasesize = spinor->dev.blocksize;
    mtd->writesize = 1;

    mtd->name = "spinor";
    mtd->type = MTD_NORFLASH;
    mtd->flags = MTD_CAP_NORFLASH;

    mtd->erase = spinor_erase;
    mtd->read = spinor_read;
    mtd->write = spinor_write;

    MTD_PR(INIT_DBG, "MTD type =0x%08x.\n", mtd->type);
    MTD_PR(INIT_DBG, "MTD flags =0x%08x.\n", mtd->flags);

    MTD_PR(INIT_DBG, "%s", mtd->name);
    MTD_PR(INIT_DBG, ":\"%s\"\n", spinor->dev.name);
    MTD_PR(INIT_DBG, "Page:%sB ", ulltostr(mtd->writesize));
    MTD_PR(INIT_DBG, "Block:%sB ", ulltostr(mtd->erasesize));
    MTD_PR(INIT_DBG, "Size:%sB\n", ulltostr(mtd->size));
}

extern struct block_operations g_dev_spinor_ops;
/*---------------------------------------------------------------------------*/
/* spinor_node_register- spinor node register */
/*---------------------------------------------------------------------------*/
int spinor_node_register(struct mtd_info *mtd)
{
    int ret = 0;
    ret = register_blockdriver("/dev/spinor", &g_dev_spinor_ops, 0755, mtd);
    //ret = register_blockdriver("/dev/spinor", &g_mtdblock_ops, 0755, mtd);
    if(ret) {
        ERR_MSG("register spinor err %d!\n",ret);
    }

    return ret;
}

/*---------------------------------------------------------------------------*/
/* spinor_init - spinor initializtion total entry */
/*---------------------------------------------------------------------------*/
int spinor_init(void)
{
    spinor_mtd = zalloc(sizeof(struct mtd_info));
    if(!spinor_mtd) {
        ERR_MSG("no mem for mtd_info!\n");
        return -1;
    }
    /* spinor chip init */
    if(spinor_chip_init(spinor_mtd)) {
        ERR_MSG("spinor chip init fail!\n");
        goto err;
    }

    /* scan spinor device entry */
    if(spinor_scan(spinor_mtd)) {
        ERR_MSG("spinor scan fail!\n");
        goto err;
    }

    /* spinor register */
    spinor_register(spinor_mtd);

    if(spinor_node_register(spinor_mtd)) {
        ERR_MSG("spinor node register fail!\n");
        return -1;
    }

    return 0;

err:
    free(spinor_mtd);
    spinor_mtd = NULL;
    return -1;
}

int spinor_wakeup_lockresume(void)
{
	return spinor_chip_resume(spinor_mtd);
}

