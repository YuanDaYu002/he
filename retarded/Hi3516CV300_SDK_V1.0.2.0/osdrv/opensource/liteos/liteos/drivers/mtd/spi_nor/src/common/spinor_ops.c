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

#include "mtd_common.h"
#include "spinor_common.h"
#include "spinor_ops.h"

static struct spinor_info *spinor;

extern int spinor_host_init(struct spinor_info *spinor);
/*---------------------------------------------------------------------------*/
/* spinor_chip_init - initializtion spinor chip and host init */
/*---------------------------------------------------------------------------*/
int spinor_chip_init(struct mtd_info *mtd)
{
    spinor = zalloc(sizeof(struct spinor_info));
    if(!spinor) {
        ERR_MSG("no mem for spinor_info!\n");
        return -1;
    }
    mtd->priv = spinor;

    if(spinor_host_init(spinor)) {
        free(spinor);
        return -1;
    }

    return 0;
}

/*---------------------------------------------------------------------------*/
/* spinor_chip_resume - spinor chip resume */
/*---------------------------------------------------------------------------*/
int spinor_chip_resume(struct mtd_info *mtd)
{
	int ret = 0;
	struct spinor_info *spinorp = mtd->priv;
	if(spinor && spinor->resume) {
		ret = spinor->resume(spinor);
	} else {
		ERR_MSG("spinor or spinor->resume is null!\n");
	}

	return ret;
}

