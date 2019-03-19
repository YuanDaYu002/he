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

#include <string.h>
#include "stdio.h"
#include "asm/errno.h"

#include <linux/mtd/mtd.h>

#include "spinor.h"
#include "mtd_common.h"
#include "spinor_common.h"

extern struct mtd_info *spinor_mtd;

/*---------------------------------------------------------------------------*/
/* hispinor_erase */
/*---------------------------------------------------------------------------*/
int hispinor_erase(unsigned long start, unsigned long size)
{
    struct erase_info opts;
    if(spinor_mtd == NULL) {
        ERR_MSG("not init spinor_mtd!!!\n");
        return -ENODEV;
    }

    //printf("[%s:%d]start=0x%x, size=0x%0x\n", __FUNCTION__, __LINE__, start, size);
    memset(&opts, 0, sizeof(opts));
    opts.addr = start;
    opts.len = size;

    return spinor_mtd->erase(spinor_mtd, &opts);
}

/*---------------------------------------------------------------------------*/
/* hispinor_write */
/*---------------------------------------------------------------------------*/
int hispinor_write(void* memaddr, unsigned long start, unsigned long size)
{
    size_t rw_size = size;
    if(spinor_mtd == NULL) {
        ERR_MSG("not init spinor_mtd!!!\n");
        return -ENODEV;
    }

    //printf("[%s:%d]start=0x%0x, size=0x%0x\n", __FUNCTION__, __LINE__, start, size);
    return spinor_mtd->write(spinor_mtd, start, rw_size, &rw_size,
            (char *)memaddr);
}

/*---------------------------------------------------------------------------*/
/* hispinor_read */
/*---------------------------------------------------------------------------*/
int hispinor_read(void* memaddr, unsigned long start, unsigned long size)
{
    size_t rw_size = size;
    if(spinor_mtd == NULL) {
        ERR_MSG("not init spinor_mtd!!!\n");
        return -ENODEV;
    }

    //printf("[%s:%d]start=0x%0x, size=0x%0x\n", __FUNCTION__, __LINE__, start, size);
    return spinor_mtd->read(spinor_mtd, start, rw_size, &rw_size,
            (char *)memaddr);
}

