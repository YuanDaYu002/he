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

/****************************************************************************
 * Included Files
 ****************************************************************************/

//#include "string.h"
#include "los_memory.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/


/****************************************************************************
 * Global Functions
 ****************************************************************************/

void *memset(void *dst, int c, size_t n)
{
    char      *tmp_addr   = (char *)dst;
    UINT32        i          = 0;
    INT32      data_32    = 0;
    INT8      *addr_8     = (INT8 *)NULL;
    INT32     *addr_32    = (INT32 *)NULL;

#if (LOSCFG_BASE_MEM_NODE_SIZE_CHECK==1) /*lint !e553*/
    UINT32 uwRet = 0;
    UINT32 uwTotalSize=0;
    UINT32 uwAvailSize=0;

    uwRet = LOS_MemNodeSizeCheck(m_aucSysMem0, dst, &uwTotalSize, &uwAvailSize);
    if (uwRet == LOS_OK && n > uwAvailSize)
    {
        return NULL;
    }
#endif

    if (n < 4)
    {
        for (i = 0; i < n; ++i)
        {
            *tmp_addr++ = (char)c;
        }
        return dst;
    }

    while ((UINT32)tmp_addr & 3)
    {
        *tmp_addr++ = (char)c;
        n--;
    }

    if (n >= 4)
    {
        for (i = 0; i < 4; ++i)
        {
            data_32 = c + (data_32 << 8);
        }

        addr_32 = (INT32 *)tmp_addr;
        do
        {
            *addr_32++ = data_32;
            n -= 4;
        } while(n >= 4);
        tmp_addr = (char *)addr_32;
    }

    addr_8  = (INT8 *)tmp_addr;
    while (n != 0)
    {
        *addr_8++ = (INT8)c;
        n--;
    }
    return dst;
}

