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

#include "string.h"
#include "errno.h"

#include "linux/mtd/mtd.h"

#include "nand.h"
#include "nand_common.h"

/*---------------------------------------------------------------------------*/
/* nand_write_yaffs_skip_bad */
/*---------------------------------------------------------------------------*/
int nand_write_yaffs_skip_bad(struct mtd_info *mtd, loff_t offset, size_t *length,
        const char *buffer)
{
    if(mtd == NULL) {
        ERR_MSG("not init nand_mtd!!!\n");
        return -ENODEV;
    }
    return mtd->write_oob(mtd, offset, *length, length, buffer);

}

/*---------------------------------------------------------------------------*/
/* nand_read_yaffs_skip_bad */
/*---------------------------------------------------------------------------*/
int nand_read_yaffs_skip_bad(struct mtd_info *mtd, loff_t offset, size_t *length,
    const char *buffer)
{
    if(mtd == NULL) {
        ERR_MSG("not init nand_mtd!!!\n");
        return -ENODEV;
    }
    return mtd->read_oob(mtd, offset, *length, length, buffer);
}

/*---------------------------------------------------------------------------*/
/* nand_write_skip_bad */
/*---------------------------------------------------------------------------*/
int nand_write_skip_bad(struct mtd_info *mtd, loff_t offset, size_t *length,
        const char *buffer)
{
    if(mtd == NULL) {
        ERR_MSG("not init nand_mtd!!!\n");
        return -ENODEV;
    }
    return mtd->write(mtd, offset, *length, length, buffer);
}

/*---------------------------------------------------------------------------*/
/* nand_read_skip_bad */
/*---------------------------------------------------------------------------*/
int nand_read_skip_bad(struct mtd_info *mtd, loff_t offset, size_t *length,
        const char *buffer)
{
    if(mtd == NULL) {
        ERR_MSG("not init nand_mtd!!!\n");
        return -ENODEV;
    }
    return mtd->read(mtd, offset, *length, length, buffer);
}

/*---------------------------------------------------------------------------*/
/* hinand_erase */
/*---------------------------------------------------------------------------*/
//int hinand_erase(loff_t start, size_t size)
int hinand_erase(unsigned long start, unsigned long size)
{
    struct erase_info opts;
    if(nand_mtd == NULL) {
        ERR_MSG("not init nand_mtd!!!\n");
        return -ENODEV;
    }

    //INFO_MSG("[%s:%d]start=0x%x, size=0x%0x\n", __FUNCTION__, __LINE__, start, size);
    memset(&opts, 0, sizeof(opts));
    opts.addr = start;
    opts.len = size;

    return nand_mtd->erase(nand_mtd, &opts);
}

/*---------------------------------------------------------------------------*/
/* hinand_write */
/*---------------------------------------------------------------------------*/
//int hinand_write(void* memaddr, loff_t start, size_t size)
int hinand_write(void* memaddr, unsigned long start, unsigned long size)
{
    size_t rw_size = size;
    if(nand_mtd == NULL) {
        ERR_MSG("not init nand_mtd!!!\n");
        return -ENODEV;
    }

    //INFO_MSG("[%s:%d]start=0x%0x, size=0x%0x\n", __FUNCTION__, __LINE__, start, size);
    return nand_write_skip_bad(nand_mtd, start, &rw_size,
            (const char *)memaddr);
}

/*---------------------------------------------------------------------------*/
/* hinand_read */
/*---------------------------------------------------------------------------*/
//int hinand_read(void* memaddr, loff_t start, size_t size)
int hinand_read(void* memaddr, unsigned long start, unsigned long size)
{
    size_t rw_size = size;
    if(nand_mtd == NULL) {
        ERR_MSG("not init nand_mtd!!!\n");
        return -ENODEV;
    }

    //INFO_MSG("[%s:%d]start=0x%0x, size=0x%0x\n", __FUNCTION__, __LINE__, start, size);
    return nand_read_skip_bad(nand_mtd, start, &rw_size,
            (const char *)memaddr);
}

/*---------------------------------------------------------------------------*/
/* hinand_yaffs_write */
/*---------------------------------------------------------------------------*/
//int hinand_yaffs_write(void* memaddr, loff_t start, size_t size)
int hinand_yaffs_write(void* memaddr, unsigned long start, unsigned long size)
{
    size_t rw_size = size;
    if(nand_mtd == NULL) {
        ERR_MSG("not init nand_mtd!!!\n");
        return -ENODEV;
    }

    //INFO_MSG("[%s:%d]start=0x%0x, size=0x%0x\n", __FUNCTION__, __LINE__, start, size);
    return nand_write_yaffs_skip_bad(nand_mtd, start, &rw_size,
            (const char *)memaddr);
}

/*---------------------------------------------------------------------------*/
/* hinand_yaffs_read */
/*---------------------------------------------------------------------------*/
//int hinand_yaffs_read(void* memaddr, loff_t start, size_t size)
int hinand_yaffs_read(void* memaddr, unsigned long start, unsigned long size)
{
    size_t rw_size = size;
    if(nand_mtd == NULL) {
        ERR_MSG("not init nand_mtd!!!\n");
        return -ENODEV;
    }

    //INFO_MSG("[%s:%d]start=0x%0x, size=0x%0x\n", __FUNCTION__, __LINE__, start, size);
    return nand_read_yaffs_skip_bad(nand_mtd, start, &rw_size,
            (const char *)memaddr);
}

/*---------------------------------------------------------------------------*/
/* hinand_addr_cal */
/*---------------------------------------------------------------------------*/
loff_t hinand_addr_cal(loff_t addr, size_t size)
{
    struct nand_info *nand = nand_mtd->priv;
    unsigned long aligned_blocksize = nand->dev.blocksize
                - (addr&(nand->dev.blocksize-1));
    while(1) {
        if (nand_mtd->block_isbad(nand_mtd, addr) != GOOD_BLOCK) {//bad block
            INFO_MSG("Skip bad block 0x%llx!\n",addr);
            addr += nand->dev.blocksize;
            if (addr + size > nand->dev.chipsize) {
                ERR_MSG("out of range!!!\n");
                return -1;
            }
        } else {//good block
            if(size > aligned_blocksize) {
                addr += aligned_blocksize;
                size -= aligned_blocksize;
                if (addr + size > nand->dev.chipsize) {
                    ERR_MSG("out of range!!!\n");
                    return -1;
                }
            } else {
                addr += size;
                size = 0;
                if (addr + size > nand->dev.chipsize) {
                    ERR_MSG("out of range!!!\n");
                    return -1;
                }
                return addr;
            }

        }
    }
}

/*---------------------------------------------------------------------------*/
/* hinand_yaffs_nand_block_isbad */
/*---------------------------------------------------------------------------*/
int hinand_yaffs_nand_block_isbad(loff_t ofs)
{
    int ret;
    if(nand_mtd == NULL) {
        ERR_MSG("not init nand_mtd!!!\n");
        return -ENODEV;
    }
    ret = nand_mtd->block_isbad(nand_mtd, ofs);
    //INFO_MSG("[%s:%d] block_isbad ofs=0x%08x ret=%0d\n", __FUNCTION__, __LINE__, (uint32_t)ofs, ret);
    return ret;
}

/*---------------------------------------------------------------------------*/
/* hinand_yaffs_nand_block_markbad */
/*---------------------------------------------------------------------------*/
int hinand_yaffs_nand_block_markbad(loff_t ofs)
{
    int ret;
    if(nand_mtd == NULL) {
        ERR_MSG("not init nand_mtd!!!\n");
        return -ENODEV;
    }

    ret = nand_mtd->block_isbad(nand_mtd, ofs);
    //INFO_MSG("[%s:%d] block_isbad ofs=0x%08x ret=%0d\n", __FUNCTION__, __LINE__, (uint32_t)ofs, ret);

    if(ret == GOOD_BLOCK)
        ret =  nand_mtd->block_markbad(nand_mtd, ofs);
    //INFO_MSG("[%s:%d] block_markbad ofs=0x%08x ret=%0d\n", __FUNCTION__, __LINE__, (uint32_t)ofs, ret);
    return ret;
}

