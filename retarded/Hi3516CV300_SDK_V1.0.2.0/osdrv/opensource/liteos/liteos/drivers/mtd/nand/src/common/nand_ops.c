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
#include "string.h"
#include "stdlib.h"
#include "linux/mtd/mtd.h"

#include "los_mux.h"

#include "nand_common.h"
#include "nand_ops.h"

static struct nand_info *nand;

/*---------------------------------------------------------------------------*/
/* nand_chip_init - initializtion nand chip and host init */
/*---------------------------------------------------------------------------*/
int nand_chip_init(struct mtd_info *mtd)
{
    nand = zalloc(sizeof(struct nand_info));
    if(!nand) {
        ERR_MSG("no mem for nand_info!\n");
        return -1;
    }
    mtd->priv = nand;

    if(nand_host_init(nand)) {
        goto err;
    }

    if(LOS_OK != LOS_MuxCreate(&nand->lock)) {
        goto err;
    }

    return 0;

err:
    free(nand);
    return -1;
}

/*---------------------------------------------------------------------------*/
/* nand_chip_resume - nand chip resume */
/*---------------------------------------------------------------------------*/
int nand_chip_resume(struct mtd_info *mtd)
{
    struct nand_info *nandp = mtd->priv;
    if(LOS_OK != LOS_MuxPost(nandp->lock)) {
        ERR_MSG("nand_host_Idle_wakeup LOS_MuxPost failed\n");
        return -1;
    }

    MTD_PR(INIT_DBG, "\t nand_host_wakeup_lockresume ok\n");

    if(nandp->resume)
        return nandp->resume(nandp);

    return 0;
}

/*---------------------------------------------------------------------------*/
/* program first page to mark bad block */
/*---------------------------------------------------------------------------*/
int nand_block_markbad(struct nand_info *nand, loff_t addr)
{
    char badflag[NAND_BB_SIZE];
    int page;
    int pagesize;
    uint8_t status;

    pagesize = nand->dev.pagesize;

    page = (int)((addr) >> nand->dev.page_shift);

    /* get device */
    if (nand->get_device(nand)) {
        return -1;
    }

    /* set first page's BB to all 0x00 to mark bad block */
    memset(badflag, 0x00, NAND_BB_SIZE);

    nand->write_buf(nand, badflag, NAND_BB_SIZE, pagesize);

    nand->program(nand, page);
    nand->op_state = NAND_PROGING;
    status = nand->read_status(nand);

    /* See if page program succeeded */
    if (status & NAND_STATUS_FAIL) {
        INFO_MSG("page[0x%0x]:program fail!\n", page);
        nand->put_device(nand);
        return -EIO;
    }

    /* release device */
    nand->put_device(nand);

    return 0;
}

/*---------------------------------------------------------------------------*/
/* read first to check whether it is a bad block */
/*---------------------------------------------------------------------------*/
int nand_block_isbad(struct nand_info *nand, loff_t addr)
{
    char bb[NAND_BB_SIZE];
    int page;

    page = (int)((addr) >> nand->dev.page_shift);

    /* get device */
    if (nand->get_device(nand)) {
        return -1;
    }

    nand->read(nand, page);
    nand->op_state = NAND_READING;
    nand->read_buf(nand, bb, NAND_BB_SIZE, nand->dev.pagesize);

    /* release device */
    nand->put_device(nand);

    if(bb[0]!=0xff || bb[1]!=0xff)//bad block
        return 1;
    return 0;
}

/*---------------------------------------------------------------------------*/
/* erase ops */
/*---------------------------------------------------------------------------*/
int nand_do_erase_ops(struct mtd_info *mtd, loff_t addr)
{
    struct nand_info *nand = mtd->priv;
    int status;
    int page;

    page = (int)(addr >> nand->dev.page_shift);

    /* get device */
    if (nand->get_device(nand)) {
        return -1;
    }

    status = nand->erase(nand, page);
    nand->op_state = NAND_ERASING;

    /* See if block erase succeeded */
    if (status & NAND_STATUS_FAIL) {
        ERR_MSG("page[0x%0x]:erase fail!\n", page);
        /* release device */
        nand->put_device(nand);
        return -EIO;
    }

    /* release device */
    nand->put_device(nand);
    return 0;
}

/*---------------------------------------------------------------------------*/
/* write data and oob */
/*---------------------------------------------------------------------------*/
int nand_do_write_ops(struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops)
{
    struct nand_info *nand = mtd->priv;
    int page, writesize, oobsize;
    uint8_t status;
    const char *datbuf = ops->datbuf;
    const char *oobbuf = ops->oobbuf;
    char badflag[NAND_BB_SIZE];

    page = (int)(to >> nand->dev.page_shift);
    writesize = ops->len;
    oobsize = ops->ooblen;

    /* get device */
    if (nand->get_device(nand)) {
        return -1;
    }

    nand->write_buf(nand, datbuf, writesize, 0);
    if(oobsize == 0) {
        memset(badflag, 0xff, NAND_BB_SIZE);
        nand->write_buf(nand, badflag, NAND_BB_SIZE, writesize);
    }
    else {
        nand->write_buf(nand, oobbuf, oobsize, writesize);
    }

    nand->program(nand, page);
    nand->op_state = NAND_PROGING;
    status = nand->read_status(nand);

    /* See if page program succeeded */
    if (status & NAND_STATUS_FAIL) {
        ERR_MSG("page[0x%0x]:program fail!\n", page);
        nand->put_device(nand);
        return -EIO;
    }

    nand->put_device(nand);

    return 0;
}

/*---------------------------------------------------------------------------*/
/* nand_do_read_ops - Send page read operation and copy read data to buf */
/*---------------------------------------------------------------------------*/
int nand_do_read_ops(struct nand_info *nand, loff_t from, struct mtd_oob_ops *ops)
{
    uint32_t page, col;
    uint32_t pagesize, oobsize;
    const char *datbuf = ops->datbuf;
    const char *oobbuf = ops->oobbuf;

    page = (unsigned int)(from >> nand->dev.page_shift);
    col = from & nand->dev.pagemask;
    pagesize = ops->len;
    oobsize = ops->ooblen;

    /* get device */
    if (nand->get_device(nand)) {
        return -1;
    }

    /* Now read the page into the buffer */
    nand->read(nand, page);
    nand->op_state = NAND_READING;
    nand->read_buf(nand, datbuf, pagesize, col);
    nand->read_buf(nand, oobbuf, oobsize, pagesize);

    /* release device */
    nand->put_device(nand);

    return 0;
}

