/*----------------------------------------------------------------------------
 * Copyright (c) <2014-2015>, <Huawei Technologies Co., Ltd>
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
#include "los_sys.h"
#include "errno.h"
#include "linux/delay.h"

#include "asm/io.h"
#include "asm/platform.h"

#define REG_RNG_BASE_ADDR       (0x120c0000)

#define  REG_RNG_CTRL_ADDR          (REG_RNG_BASE_ADDR + 0x200)
#define  REG_RNG_NUMBER_ADDR        (REG_RNG_BASE_ADDR + 0x204)
#define  REG_RNG_STAT_ADDR          (REG_RNG_BASE_ADDR + 0x208)
#define  REG_RNG_ERR0_ADDR          (REG_RNG_BASE_ADDR + 0x20c)

#define PERI_CRG64  0x100
#define TRNG_CKEN   (0x1<<3)

int hi_random_hw_init(void)
{
    unsigned int rngstat = 0;

    rngstat = readl(CRG_REG_BASE + PERI_CRG64);
    rngstat |= TRNG_CKEN;
    writel(rngstat, CRG_REG_BASE + PERI_CRG64);
    
    rngstat = readl(REG_RNG_CTRL_ADDR);
    rngstat &= 0xfffffffc;
    rngstat |= 0x2; /* select rng source 0x2, but 0x03 is ok too */
    rngstat |= 0x00009000;
    rngstat |= 0x000000a0;
    writel(rngstat,REG_RNG_CTRL_ADDR);
    return ENOERR;
}

int hi_random_hw_deinit(void)
{
    unsigned int rngstat = 0;
    rngstat = readl(REG_RNG_CTRL_ADDR);
    rngstat &= 0xfffffffc;
    writel(rngstat,REG_RNG_CTRL_ADDR);
    
    rngstat = readl(CRG_REG_BASE + PERI_CRG64);
    rngstat &= ~TRNG_CKEN;
    writel(rngstat, CRG_REG_BASE + PERI_CRG64);
    return ENOERR;
}

#define TRNG_FIFO_DATA_CNT_SHIFT 8
#define TRNG_FIFO_DATA_CNT_MASK (0xff<<TRNG_FIFO_DATA_CNT_SHIFT)

int hi_random_hw_getinteger(unsigned int *pResult)
{
    unsigned int count = 0;
    unsigned int rngstat = 0;
    unsigned int timeout = 0xffffffff;

    /* low 3bit(RNG_data_count[2:0]), indicate how many RNGs in the fifo is available now . max is 0x4*/
    while(1) {
        rngstat = readl(REG_RNG_STAT_ADDR);
        count  = (rngstat & TRNG_FIFO_DATA_CNT_MASK)>>TRNG_FIFO_DATA_CNT_SHIFT;
        if(count>0 || timeout==0)
            break;
        timeout--;
    }

    if(count > 0)
    {
        *pResult = readl(REG_RNG_NUMBER_ADDR);
        return ENOERR;
    }
    else
    {
        return -EINVAL;
    }
}

int hi_random_hw_getnumber(char *buffer, size_t buflen)
{
    unsigned int rngstat = 0;
    unsigned int count = 0;
    unsigned int quit = 0;
    size_t len = buflen;
    char *buf = buffer;

    if(len % sizeof(unsigned int))
        return -EINVAL;

    while(!quit)
    {
        /* low 3bit(RNG_data_count[2:0]), indicate how many RNGs in the fifo is available now . max is 0x4*/
        rngstat = readl(REG_RNG_STAT_ADDR);
        count = (rngstat & TRNG_FIFO_DATA_CNT_MASK)>>TRNG_FIFO_DATA_CNT_SHIFT;
        if(count > 0)
        {
            while(len > 0)
            {
                len -= sizeof(unsigned int);
                *(unsigned int *)buf = readl(REG_RNG_NUMBER_ADDR);
                buf += sizeof(unsigned int);
                count--;
                if(len == 0)
                {
                    quit = 1;
                    break;
                }
                if(count<=0)
                {
                    break;
                }
            }
        }

        if(!quit)
        {
            msleep(1);
        }
    }

    return ENOERR;
}
