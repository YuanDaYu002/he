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

 #ifndef _HWLITEOS_SHELL_SHFIFO_H
 #define _HWLITEOS_SHELL_SHFIFO_H

#include "los_typedef.h"
#include "shell.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


#define                 SH_FIFO_LEN                 1024

typedef struct tagFifo
{
    CHAR    acFifoBuf[SH_FIFO_LEN];
    UINT32 uwFifoIn;                    //存入数据标识
    UINT32 uwFifoOut;                 //取出数据标识
    UINT32 uwFifoNum;               //fifo中剩余的数
}FIFO_T;

//extern FIFO_T g_stRxFifo;
//extern FIFO_T g_stWxFifo;
//extern UINT32 shFifoFlag;

extern void shFifoInit(FIFO_T * pstFifo);
extern UINT32 shFifoGet(FIFO_T * pstFifo, UINT8 * pcBuf, UINT32 uwlen);
extern UINT32 shFifoPut(FIFO_T * pstFifo, UINT8 * pcBuf, UINT32 uwlen);
extern UINT32 shFifoGetByte(FIFO_T * pstFifo, UINT8 * ch);
extern UINT32 shFifoPutByte(FIFO_T * pstFifo, UINT8 ch);
extern UINT32 shFifoGetNum(FIFO_T * pstFifo);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif