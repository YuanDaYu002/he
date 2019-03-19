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

#include "asm/errno.h"
#include "linux/mtd/mtd.h"

#include "mtd_common.h"
#include "spinor_common.h"
#include "spinor_ids.h"

/*---------------------------------------------------------------------------*/
/* spinor_get_dev_id - Get current spinor device Identify code */
/*---------------------------------------------------------------------------*/
static int spinor_get_dev_id(struct spinor_info *spinor)
{
    char tmp_ids[SPI_NOR_MAX_ID_LEN]={0};
    char *ids = spinor->dev.id;

    /* Clean out ID array in spinor_dev_info */
    memset(ids, 0, SPI_NOR_MAX_ID_LEN);

    /* Send command for read spinor device ID */
    spinor->read_id(spinor, ids);
    if (!(ids[0] | ids[1]) || ((ids[0] & ids[1]) == 0xFF)) {
        DBG_MSG("Spi(cs%d) have no device.\n", spinor->cur_cs);
        return -ENODEV;
    }

    /* Try again to make sure this IDs isn't random data */
    spinor->read_id(spinor, tmp_ids);
    if (memcmp(ids, tmp_ids, sizeof(tmp_ids))) {
        ERR_MSG("Second ID:%02x %02x != %02x %02x\n",
        tmp_ids[0], tmp_ids[1], ids[0], ids[1]);
        return -ENODEV;
    }

    return 0;
}

#define MAX_CS_NUM 2
/*---------------------------------------------------------------------------*/
/* spinor_get_dev_info - Get spinor device information */
/*---------------------------------------------------------------------------*/
static int spinor_get_dev_info(struct spinor_info *spinor)
{
    for(int i=0;i<MAX_CS_NUM;i++) {
        spinor->cur_cs = i;
        if (spinor_get_dev_id(spinor)) {
            INFO_MSG("Cs[%0d] have no device!!!\n", spinor->cur_cs);
            continue;
        }

        if (!spinor_get_dev_info_by_id(spinor)) {
            INFO_MSG("No device match cs[%0d]'s ID!!!\n", spinor->cur_cs);
            continue;
        }

        spinor->numchips++;
        break;
    }
    if (!spinor->numchips)
        return -ENODEV;
    return 0;
}

/*---------------------------------------------------------------------------*/
/* spinor_scan */
/*---------------------------------------------------------------------------*/
int spinor_scan(struct mtd_info *mtd)
{
     struct spinor_info *spinor = mtd->priv;

     if(spinor_get_dev_info(spinor))
         return -1;

     MTD_PR(INIT_DBG, "spinor name:%s\n",spinor->dev.name);
     MTD_PR(INIT_DBG, "spinor id_len:0d%0d.\n",spinor->dev.id_len);
     MTD_PR(INIT_DBG, "spinor blocksize:0x%0x.\n",spinor->dev.blocksize);
     MTD_PR(INIT_DBG, "spinor chipsize:0x%0llx.\n",spinor->dev.chipsize);
     MTD_PR(INIT_DBG, "spinor numchips:0d%0d.\n",spinor->numchips);

     return 0;
}

