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

#include "match_table.h"

#include "nand_common.h"
#include "host_common.h"
#include "hifmc_common.h"

/*****************************************************************************/

static struct match_reg_type page_type2reg[] = {
    {
        hifmc_pagesize_2K, NAND_PAGE_2K,
    }, {
        hifmc_pagesize_4K, NAND_PAGE_4K,
    }, {
        hifmc_pagesize_8K, NAND_PAGE_8K,
    }, {
        hifmc_pagesize_16K, NAND_PAGE_16K,
    }
};

enum hifmc_page_reg hifmc_page_type2reg(int type)
{
    return type2reg(page_type2reg, ARRAY_SIZE(page_type2reg), type, 0);
}

int hifmc_page_reg2type(enum hifmc_page_reg reg)
{
    return reg2type(page_type2reg, ARRAY_SIZE(page_type2reg), reg, 0);
}
/*****************************************************************************/

static struct match_reg_type ecc_type2reg[] = {
    {
        hifmc_ecc_0bit, NAND_ECC_0BIT,
    }, {
        hifmc_ecc_8bit, NAND_ECC_8BIT_1K,
    }, {
        hifmc_ecc_24bit, NAND_ECC_24BIT_1K,
    }, {
        hifmc_ecc_40bit, NAND_ECC_40BIT_1K,
    }, {
        hifmc_ecc_64bit, NAND_ECC_64BIT_1K,
    }
};

enum hifmc_ecc_reg hifmc_ecc_type2reg(int type)
{
    return type2reg(ecc_type2reg, ARRAY_SIZE(ecc_type2reg), type, 0);
}

int hifmc_ecc_reg2type(enum hifmc_ecc_reg reg)
{
    return reg2type(ecc_type2reg, ARRAY_SIZE(ecc_type2reg), reg, 0);
}

