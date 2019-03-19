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

#include "los_mux.h"

#include "asm/delay.h"

#include "nand_common.h"

/*---------------------------------------------------------------------------*/
/* nand_get_device */
/*---------------------------------------------------------------------------*/
static int nand_get_device(struct nand_info *nand)
{
    if(LOS_OK!=LOS_MuxPend(nand->lock, LOS_WAIT_FOREVER))
        return -EBUSY;
    else
        return 0;
}

/*---------------------------------------------------------------------------*/
/* nand_put_device */
/*---------------------------------------------------------------------------*/
static void nand_put_device(struct nand_info *nand)
{
    (void)LOS_MuxPost(nand->lock);
}

extern struct nand_dev_info *nand_get_dev_info_by_id(struct nand_info *nand);
/*---------------------------------------------------------------------------*/
/* nand_check_func - Check and set default for nand supply function */
/*---------------------------------------------------------------------------*/
static void nand_check_func(struct nand_info *nand)
{
    if (!nand->get_device)
        nand->get_device = nand_get_device;

    if (!nand->put_device)
        nand->put_device = nand_put_device;
}

/*---------------------------------------------------------------------------*/
/* nand_get_dev_id - Get current nand device Identify code */
/*---------------------------------------------------------------------------*/
int nand_get_dev_id(struct nand_info *nand)
{
    char tmp_ids[NAND_MAX_ID_LEN];
    char *ids = nand->dev.id;

    /* Clean out ID array in nand_dev_info */
    memset(ids, 0, NAND_MAX_ID_LEN);

    /* Reset chip, required by some chips */
    //nand->reset(nand);

    /* Send command for read nand device ID */
    nand->read_id(nand, ids);

    /* Try again to make sure this IDs isn't random data */
    nand->read_id(nand, tmp_ids);
    if (tmp_ids[0] != ids[0] || tmp_ids[1] != ids[1]) {
        ERR_MSG("Second ID:%02x %02x did not match first:%02x" \
            " %02x\n", tmp_ids[0], tmp_ids[1], ids[0], ids[1]);
        return -ENODEV;
    }

    return 0;
}

#define MAX_CS_NUM 2
/*---------------------------------------------------------------------------*/
/* nand_get_dev_info - Get Nand device information */
/*---------------------------------------------------------------------------*/
static int nand_get_dev_info(struct nand_info *nand)
{
    for(int i=0;i<MAX_CS_NUM;i++) {
        nand->cur_cs = i;
        if (nand_get_dev_id(nand)) {
            INFO_MSG("Cs[%0d] have no device!!!\n", nand->cur_cs);
            continue;
        }

        if (!nand_get_dev_info_by_id(nand)) {
            INFO_MSG("No device match cs[%0d]'s ID!!!\n", nand->cur_cs);
            continue;
        }

    nand->numchips++;
        break;
    }

    if (!nand->numchips)
        return -ENODEV;

    return nand->oob_resize(nand);
}

/*---------------------------------------------------------------------------*/
/* nand_scan - Scan nand device entry */
/*---------------------------------------------------------------------------*/
int nand_scan(struct mtd_info *mtd)
{
    struct nand_info *nand = mtd->priv;

    /* Check nand supply function */
    nand_check_func(nand);

    if(nand_get_dev_info(nand))
        return -1;

    MTD_PR(INIT_DBG, "NAND name:%s\n",nand->dev.name);
    MTD_PR(INIT_DBG, "NAND id_len:0d%0d.\n",nand->dev.id_len);
    MTD_PR(INIT_DBG, "NAND oobsize:0d%0d.\n",nand->dev.oobsize);
    MTD_PR(INIT_DBG, "NAND pagesize:0d%0d.\n",nand->dev.pagesize);
    MTD_PR(INIT_DBG, "NAND page_shift:0d%0d.\n",nand->dev.page_shift);
    MTD_PR(INIT_DBG, "NAND pagemask:0x%0x.\n",nand->dev.pagemask);
    MTD_PR(INIT_DBG, "NAND blocksize:0x%0x.\n",nand->dev.blocksize);
    MTD_PR(INIT_DBG, "NAND block_shift:0d%0d.\n",nand->dev.block_shift);
    MTD_PR(INIT_DBG, "NAND blockmask:0x%0x.\n",nand->dev.blockmask);
    MTD_PR(INIT_DBG, "NAND chipsize:0x%0llx.\n",nand->dev.chipsize);
    MTD_PR(INIT_DBG, "NAND chip_shift:0d%0d.\n",nand->dev.chip_shift);
    MTD_PR(INIT_DBG, "NAND numchips:0d%0d.\n",nand->numchips);

    return 0;
}

