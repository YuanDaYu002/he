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

#include "nand.h"
#include "nand_scan.h"
#include "nand_ops.h"

/*---------------------------------------------------------------------------*/
struct mtd_info *nand_mtd = NULL;

/*---------------------------------------------------------------------------*/
/* nand erase, register to mtd->erase */
/*---------------------------------------------------------------------------*/
static int nand_erase(struct mtd_info *mtd, struct erase_info *ops)
{
    int ret = 0;
    struct nand_info *nand = mtd->priv;
    loff_t addr = ops->addr;
    loff_t len = ops->len;
    ops->fail_addr = MTD_FAIL_ADDR_UNKNOWN;

    /* Start address must align on block boundary */
    if (addr & (mtd->erasesize -1)) {
        ERR_MSG("Unaligned erase address!\n");
        return -EINVAL;
    }

    if ((len + addr) > mtd->size) {
        ERR_MSG("Out of range!\n");
        return -EINVAL;
    }

    if (len < mtd->erasesize) {
        WARN_MSG("Erase size 0x%08x smaller than one "    \
                "erase block 0x%08x\n!", len, mtd->erasesize);
        return -EINVAL;
    }

    for(; addr < ops->addr + len; addr += mtd->erasesize) {
        if(nand_block_isbad(nand, addr)) {
            INFO_MSG("Skipping bad block at "
                    "0x%08x\n", (uint32_t)addr);
            continue;
        }

        ret = nand_do_erase_ops(mtd, addr);

        if(ret != 0) {
            MTD_PR(ER_DBG, "NAND erase failure: %d\n",ret);

            nand_block_markbad(nand, addr);
            MTD_PR(ER_DBG, "Block at 0x%08x is marked" \
                    " bad block\n",    (uint32_t)addr);

            ops->fail_addr = addr;
            return -EIO;
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/* get_len_incl_bad - get all len include bad block*/
/*---------------------------------------------------------------------------*/
static size_t get_len_incl_bad(struct mtd_info *mtd, loff_t offset,
                const size_t length)
{
    size_t len_incl_bad = 0;
    size_t len_excl_bad = 0;
    size_t block_len;
    struct nand_info *nand = mtd->priv;

    MTD_PR(RD_DBG, "start get real len include bad block.\n");

    while (len_excl_bad < length) {
        block_len = nand->dev.blocksize
            - (offset & (nand->dev.blocksize - 1));

        if (!nand_block_isbad(nand,
                offset & ~((loff_t)nand->dev.blocksize - 1)))
            len_excl_bad += block_len;

        len_incl_bad += block_len;
        offset       += block_len;

        if (offset > mtd->size)
            break;
    }
    MTD_PR(RD_DBG, "finished get real len include bad block.\n");

    return len_incl_bad;
}

/*---------------------------------------------------------------------------*/
/* nand write - register to mtd.write */
/*---------------------------------------------------------------------------*/
static int nand_write(struct mtd_info *mtd, loff_t addr, size_t len,
            size_t *retlen, const char *buffer)
{
    struct nand_info *nand = mtd->priv;
    size_t left_to_write;
    size_t len_incl_bad;
    struct mtd_oob_ops ops = {0};
    size_t len_data;
    int    i, ret;
    loff_t to = addr;
    const char * buf = buffer;

    *retlen = len;

    if (to & (mtd->writesize - 1)) {
        ERR_MSG("Write address not page aligned.\n");
        return -EINVAL;
    }

    if (len % (mtd->writesize) != 0) {
        ERR_MSG("Attempt to write non aligned data, read length should "\
            "be aligned with (pagesize), length:%d "
            "pagesize:%d\n",
            (uint32_t)len, mtd->writesize);
        return -EINVAL;
    }

    len_data = len;

    len_incl_bad = get_len_incl_bad(mtd, to, len_data);

    if ((to + len_incl_bad) > mtd->size) {
        ERR_MSG("Attempt to write outside the flash area.\n");
        return -EINVAL;
    }

    MTD_PR(WR_DBG, "len_data=0x%0x,len_incl_bad=0x%0x.\n",len_data, len_incl_bad);

    if (len_incl_bad == len_data) {
        for(i = 0; i < len_data / mtd->writesize; i++) {
            ops.datbuf = (const char *)buf + i * (mtd->writesize);
            ops.len = mtd->writesize;
            ops.ooblen = 0;

            ret = nand_do_write_ops(mtd, to + i * mtd->writesize, &ops);
            if (ret != 0) {
                ERR_MSG("NAND write to to %08x failed %d\n",
                        (uint32_t)to, ret);
                return -EIO;
            }
        }
        MTD_PR(WR_DBG, "NAND write finished.\n");
        return 0;
    }

    left_to_write = len_data;

    while (left_to_write > 0) {
        size_t block_offset = to & (mtd->erasesize - 1);
        size_t write_size;

        if (nand_block_isbad(nand, to & ~((loff_t)mtd->erasesize - 1))) {
            INFO_MSG("Skip bad block 0x%08x\n",
                    (uint32_t)(to & ~((loff_t)mtd->erasesize - 1)));
            to += mtd->erasesize - block_offset;
            continue;
        }

        if (left_to_write < (mtd->erasesize - block_offset))
            write_size = left_to_write;
        else
            write_size = mtd->erasesize - block_offset;

        for (i = 0; i < write_size / mtd->writesize; i++) {
            ops.datbuf = (const char *)buf;
            ops.len = mtd->writesize;
            ops.ooblen = 0;

            ret = nand_do_write_ops(mtd, to, &ops);
            if (ret != 0) {
                ERR_MSG("NAND write to offset %08x failed %d\n",
                        (uint32_t)to, ret);
                return -EIO;
            }

            buf += mtd->writesize;
            left_to_write -= mtd->writesize;
            to += mtd->writesize;
        }
    }
    MTD_PR(WR_DBG, "NAND write finished.\n");
    return 0;
}


/*---------------------------------------------------------------------------*/
/* nand write include oob - register to mtd->write_oob */
/*---------------------------------------------------------------------------*/
static int nand_write_oob(struct mtd_info *mtd, loff_t addr, size_t len,
    size_t *retlen, const char *buffer)
{
    struct nand_info *nand = mtd->priv;
    size_t left_to_write;
    size_t len_incl_bad;
    struct mtd_oob_ops ops = {0};
    size_t len_data;
    int    i, ret;
    loff_t to = addr;
    const char * buf = buffer;

    *retlen = len;

    if (to & (mtd->writesize - 1)) {
        ERR_MSG("Write address not page aligned.\n");
        return -EINVAL;
    }

    if ((len % (mtd->writesize + mtd->oobsize)) != 0) {
        ERR_MSG("Attempt to write non aligned data, read length should "\
            "be aligned with (pagesize + oobsize), length:%d "
            "pagesize:%d oobsize:%d\n",
            (uint32_t)len, mtd->writesize, mtd->oobsize);
        return -EINVAL;
    }

    len_data = (len / (mtd->writesize + mtd->oobsize))* mtd->writesize;

    len_incl_bad = get_len_incl_bad(mtd, to, len_data);

    if ((to + len_incl_bad) > mtd->size) {
        ERR_MSG("Attempt to write outside the flash area.\n");
        return -EINVAL;
    }

    MTD_PR(WR_DBG, "len_data=0x%0x,len_incl_bad=0x%0x.\n",len_data, len_incl_bad);

    if (len_incl_bad == len_data) {
        for(i = 0; i < len_data / mtd->writesize; i++) {
            ops.datbuf = (const char *)buf
                + i * (mtd->writesize + mtd->oobsize);
            ops.oobbuf = (const char *)buf
                + i * (mtd->writesize + mtd->oobsize)
                + mtd->writesize;
            ops.len = mtd->writesize;
            ops.ooblen = mtd->oobsize;

            ret = nand_do_write_ops(mtd, to + i * mtd->writesize, &ops);
            if (ret != 0) {
                ERR_MSG("NAND write to to %08x failed %d\n",
                        (uint32_t)to, ret);
                return -EIO;
            }
        }
        MTD_PR(WR_DBG, "NAND write with OOB finished.\n");
        return 0;
    }

    left_to_write = len_data;

    while (left_to_write > 0) {
        size_t block_offset = to & (mtd->erasesize - 1);
        size_t write_size;

        if (nand_block_isbad(nand, to
                    & ~((loff_t)mtd->erasesize - 1))) {
            INFO_MSG("Skip bad block 0x%08x\n",
                    (uint32_t)(to & ~((loff_t)mtd->erasesize - 1)));
            to += mtd->erasesize - block_offset;
            continue;
        }

        if (left_to_write < (mtd->erasesize - block_offset))
            write_size = left_to_write;
        else
            write_size = mtd->erasesize - block_offset;

        for (i = 0; i < write_size / mtd->writesize; i++) {
            ops.datbuf = (const char *)buf;
            ops.oobbuf = (const char *)buf + mtd->writesize;
            ops.len = mtd->writesize;
            ops.ooblen = mtd->oobsize;

            ret = nand_do_write_ops(mtd, to, &ops);
            if (ret != 0) {
                ERR_MSG("NAND write to offset %08x failed %d\n",
                        (uint32_t)to, ret);
                return -EIO;
            }

            buf += mtd->writesize + mtd->oobsize;
            left_to_write -= mtd->writesize;
            to += mtd->writesize;
        }
    }
    MTD_PR(WR_DBG, "NAND write with OOB finished.\n");
    return 0;
}

/*---------------------------------------------------------------------------*/
/* nand_read - [Internal] Read nand device data to buffer             */
/*---------------------------------------------------------------------------*/
static int nand_read(struct mtd_info *mtd, loff_t addr, size_t len,
            size_t *retlen, const char *buf)
{
    struct nand_info *nand = mtd->priv;
    int ret;
    ssize_t left_to_read = len;
    size_t len_incl_bad, col;
    struct mtd_oob_ops ops = {0};
    ops.ooblen = 0;
    ops.datbuf = (const char *)buf;
    loff_t from = addr;

    *retlen = len;

    len_incl_bad = get_len_incl_bad(mtd, from, len);

    if ((from + len_incl_bad) > mtd->size) {
        ERR_MSG("Attempt to read outside the flash area.\n");
        return -EINVAL;
    }
    MTD_PR(RD_DBG, "len=0x%0x,len_incl_bad=0x%0x.\n",len, len_incl_bad);

    col = from & nand->dev.pagemask;

    if (len_incl_bad == len) {
        left_to_read = len_incl_bad;
        if(col != 0) {
            if(left_to_read <= mtd->writesize - col) {
                ops.len = left_to_read;
            }
            else {
                ops.len = mtd->writesize - col;
            }
            ret = nand_do_read_ops(nand, from, &ops);
            if (ret != 0) {
                ERR_MSG("NAND read to from %08x failed %d\n",
                        (uint32_t)from, ret);
                return -EIO;
            }

            from += ops.len;
            left_to_read -= ops.len;
            ops.datbuf += ops.len;
            //col = 0;
        }

        while (left_to_read > 0) {
            if(left_to_read >= mtd->writesize) {
                ops.len = mtd->writesize;
            }
            else {
                ops.len = left_to_read;
            }
            ret = nand_do_read_ops(nand, from, &ops);
            if (ret != 0) {
                ERR_MSG("NAND read to from %08x failed %d\n",
                        (uint32_t)from, ret);
                return -EIO;
            }

            ops.datbuf += ops.len;
            from += ops.len;
            left_to_read -= ops.len;
        }
        return 0;
    }

    while (left_to_read > 0) {
        size_t block_from = from & (mtd->erasesize - 1);
        ssize_t read_size;

        if (nand_block_isbad(nand, from &
                    ~((loff_t)mtd->erasesize - 1))) {
            INFO_MSG("Skip bad block 0x%08x\n",
                    (uint32_t)(from & ~((loff_t)mtd->erasesize - 1)));
            from = from + mtd->erasesize;
            continue;
        }

        if (left_to_read < (mtd->erasesize - block_from)) {
            read_size = left_to_read;
        }
        else {
            read_size = mtd->erasesize - block_from;
        }
        if(col != 0) {
            if(left_to_read <= mtd->writesize - col) {
                ops.len = left_to_read;
            }
            else {
                ops.len = mtd->writesize - col;
            }
            ret = nand_do_read_ops(nand, from, &ops);
            if (ret != 0) {
                ERR_MSG("NAND read to from %08x failed %d\n",
                        (uint32_t)from, ret);
                return -EIO;
            }

            from += ops.len;
            left_to_read -= ops.len;
            read_size -= ops.len;
            ops.datbuf += ops.len;
            col = 0;
        }
        while(read_size > 0) {
            if(read_size >= mtd->writesize) {
                ops.len = mtd->writesize;
            }
            else {
                ops.len = read_size;
            }
            ret = nand_do_read_ops(nand, from, &ops);

            if (ret != 0) {
                ERR_MSG("NAND read to from %08x failed %d\n",
                        (uint32_t)from, ret);
                return -EIO;
            }

            ops.datbuf += ops.len;
            from += ops.len;
            left_to_read -= ops.len;
            read_size -= ops.len;
        }
    }

    return 0;
}

/*---------------------------------------------------------------------------*/
/* nand read with OOB */
/*---------------------------------------------------------------------------*/
static int nand_read_oob(struct mtd_info *mtd, loff_t addr, size_t len,
        size_t *retlen, const char *buf)
{
    struct nand_info *nand = mtd->priv;
    int ret;
    size_t left_to_read;
    size_t len_incl_bad;
    struct mtd_oob_ops ops = {0};
    int i;
    size_t len_data;
        loff_t from = addr;

    *retlen = len;

    if (from & (mtd->writesize - 1)) {
        ERR_MSG("Attempt to read non page aligned data, from %08x.\n",
            (uint32_t)from);
        return -EINVAL;
    }
    if ((len % (mtd->writesize  + mtd->oobsize)) != 0) {
        ERR_MSG("Attempt to read non aligned data, read length should "\
            "be aligned with (pagesize + oobsize), length:%d "
            "pagesize:%d oobsize:%d\n",
            (uint32_t)len, mtd->writesize, mtd->oobsize);
        return -EINVAL;
    }

    len_data = len / (mtd->writesize + mtd->oobsize)* mtd->writesize;
    len_incl_bad = get_len_incl_bad(mtd, from, len_data);

    if ((from + len_incl_bad) > mtd->size) {
        ERR_MSG("Attempt to read outside the flash area.\n");
        return -EINVAL;
    }
    MTD_PR(RD_DBG, "len_data=0x%0x,len_incl_bad=0x%0x.\n",len_data, len_incl_bad);

    if (len_incl_bad == len_data) {
        for (i = 0; i < len_data / mtd->writesize; i++) {
            ops.datbuf = (const char *)buf +
                 i*(mtd->writesize + mtd->oobsize);
            ops.oobbuf = (const char *)buf +
                 i*(mtd->writesize + mtd->oobsize) +
                 mtd->writesize;
            ops.len = mtd->writesize;
            ops.ooblen = mtd->oobsize;

            ret = nand_do_read_ops(nand, from + i*mtd->writesize, &ops);
            if (ret != 0) {
                ERR_MSG("NAND read to from %08x failed %d\n",
                    (uint32_t)from + i*mtd->writesize, ret);
                return -EIO;
            }

            len -= mtd->writesize + mtd->oobsize;
        }

        return 0;
    }

    left_to_read = len_data;

    while (left_to_read > 0) {
        size_t block_from = from & (mtd->erasesize - 1);
        size_t read_size;

        if (nand_block_isbad(nand, from &
             ~((loff_t)mtd->erasesize - 1))) {
            INFO_MSG("Skip bad block 0x%08x\n",
                (uint32_t)(from & ~((loff_t)mtd->erasesize - 1)));
            from += mtd->erasesize - block_from;
            continue;
        }

        if (left_to_read < (mtd->erasesize - block_from))
            read_size = left_to_read;
        else
            read_size = mtd->erasesize - block_from;

        for (i = 0; i < read_size / mtd->writesize; i++) {
            ops.datbuf = (const char *)buf;
            ops.oobbuf = (const char *)buf + mtd->writesize;
            ops.len = mtd->writesize;
            ops.ooblen = mtd->oobsize;

            ret = nand_do_read_ops(nand, from, &ops);
            if (ret != 0) {
                ERR_MSG("NAND read to from %08x failed %d\n",
                (uint32_t)from, ret);
                return -EIO;
            }

            buf += mtd->writesize + mtd->oobsize;
            left_to_read -= mtd->writesize;
            from += mtd->writesize;
            len -= mtd->writesize + mtd->oobsize;
        }
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
/* block_isbad */
/*----------------------------------------------------------------------------*/
static int block_isbad (struct mtd_info *mtd, loff_t ofs)
{
    struct nand_info *nand = mtd->priv;
    return nand_block_isbad(nand, ofs);
}

/*----------------------------------------------------------------------------*/
/* block_markbad */
/*----------------------------------------------------------------------------*/
static int block_markbad (struct mtd_info *mtd, loff_t ofs)
{
    struct nand_info *nand = mtd->priv;
    return nand_block_markbad(nand, ofs);
}

#define YAFFS_OOB_SIZE 32
/*---------------------------------------------------------------------------*/
/* nand_register - nand operations func register to mtd_info struct          */
/*---------------------------------------------------------------------------*/
static void nand_register(struct mtd_info *mtd)
{
    struct nand_info *nand;

    nand = mtd->priv;

    mtd->size = nand->dev.chipsize;
    mtd->erasesize = nand->dev.blocksize;
    mtd->writesize = nand->dev.pagesize;
    mtd->oobsize = YAFFS_OOB_SIZE;
    mtd->name = "nand";
    mtd->type = MTD_NANDFLASH;
    mtd->flags = MTD_CAP_NANDFLASH;

    mtd->erase = nand_erase;
    mtd->read_oob = nand_read_oob;
    mtd->write_oob = nand_write_oob;
    mtd->read = nand_read;
    mtd->write = nand_write;
    mtd->block_isbad = block_isbad;
    mtd->block_markbad = block_markbad;

    MTD_PR(INIT_DBG, "MTD type =0x%08x.\n", mtd->type);
    MTD_PR(INIT_DBG, "MTD flags =0x%08x.\n", mtd->flags);

    MTD_PR(INIT_DBG, "%s", mtd->name);
    MTD_PR(INIT_DBG, ":\"%s\"\n", nand->dev.name);
    MTD_PR(INIT_DBG, "MTD USE OOB:%sB ", ulltostr(mtd->oobsize));
    MTD_PR(INIT_DBG, "Page:%sB ", ulltostr(mtd->writesize));
    MTD_PR(INIT_DBG, "Block:%sB ", ulltostr(mtd->erasesize));
    MTD_PR(INIT_DBG, "Size:%sB\n", ulltostr(mtd->size));
}

extern struct block_operations g_dev_nand_ops;
/*---------------------------------------------------------------------------*/
/* nand_node_register- nand node register */
/*---------------------------------------------------------------------------*/
int nand_node_register(struct mtd_info *mtd)
{
    int ret = 0;
    ret = register_blockdriver("/dev/nand", &g_dev_nand_ops, 0755, mtd);
    if(ret) {
        INFO_MSG("register nand err %d\n",ret);
    }

    return ret;
}

/*---------------------------------------------------------------------------*/
/* nand_init - nand initializtion total entry */
/*---------------------------------------------------------------------------*/
int nand_init(void)
{
    nand_mtd = zalloc(sizeof(struct mtd_info));
    if(!nand_mtd) {
        ERR_MSG("no mem for mtd_info!\n");
        return -1;
    }
    /* nand chip init */
    if(nand_chip_init(nand_mtd)) {
        ERR_MSG("nand chip init fail!\n");
        goto err;
    }

    /* scan nand device entry */
    if(nand_scan(nand_mtd)) {
        ERR_MSG("nand scan fail!\n");
        goto err;
    }

    /* nand register */
    nand_register(nand_mtd);

    if(nand_node_register(nand_mtd)) {
        ERR_MSG("nand node register fail!\n");
        return -1;
    }

    return 0;

err:
    free(nand_mtd);
    nand_mtd = NULL;
    return -1;
}

/*---------------------------------------------------------------------------*/
/* nand_wakeup_lockresume - nand resume */
/*---------------------------------------------------------------------------*/
int nand_wakeup_lockresume(void)
{
	return nand_chip_resume(nand_mtd);
}

