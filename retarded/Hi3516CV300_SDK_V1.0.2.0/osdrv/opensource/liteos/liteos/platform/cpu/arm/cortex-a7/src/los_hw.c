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

#include "los_base.h"
#include "los_hwi.h"
#include "los_hw.ph"
#include "los_task.ph"
#include "los_memory.h"
#include "los_membox.h"
#include "los_priqueue.ph"
#include "asm/dma.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

VOID osSchedule(VOID)
{
    osTaskSchedule();
}
VOID LOS_Schedule(VOID)
{
    UINTPTR uvIntSave;
    uvIntSave = LOS_IntLock();
    g_stLosTask.pstNewTask = LOS_DL_LIST_ENTRY(osPriqueueTop(), LOS_TASK_CB, stPendList);/*lint !e413*/
    if (g_stLosTask.pstRunTask != g_stLosTask.pstNewTask)
    {
        if ((!g_usLosTaskLock) && (!OS_INT_ACTIVE))
        {
            (VOID)LOS_IntRestore(uvIntSave);
            osTaskSchedule();
            return;
        }
    }

    (VOID)LOS_IntRestore(uvIntSave);
}

VOID osTaskExit(void)
{
    __asm__ __volatile__("swi  0");
}

/*****************************************************************************
 Function    : osTskStackInit
 Description : task stack initialization
 Input       : pfnTaskEntry -- task handler
               uwStackSize  -- task stack size
               pTopStack    -- stack top of task (low address)
 Output      : None
 Return      : OS_SUCCESS on success or error code on failure
 *****************************************************************************/
VOID *osTskStackInit(UINT32 uwTaskID, UINT32 uwStackSize, VOID *pTopStack)
{
    UINT32 uwIdx;
    TSK_CONTEXT_S  *pstContext;

    /*initialize the task stack, write magic num to stack top*/
    for (uwIdx = 0; uwIdx < (uwStackSize/sizeof(UINT32)); uwIdx++)
    {
        *((UINT32 *)pTopStack + uwIdx) = OS_TASK_STACK_INIT;
    }
    *((UINT32 *)(pTopStack)) = OS_TASK_MAGIC_WORD;

    pstContext    = (TSK_CONTEXT_S *)(((UINT32)pTopStack + uwStackSize) - sizeof(TSK_CONTEXT_S));

    /*initialize the task context*/
    pstContext->uwPC    = (UINT32)osTaskEntry;
    pstContext->uwLR    = (UINT32)osTaskExit;             /* LR should be kept, to distinguish it's THUMB instruction or ARM instruction*/
    pstContext->auwR[0 ] = uwTaskID;      /* R0  */
    pstContext->auwR[1 ] = 0x01010101;      /* R1  */
    pstContext->auwR[2 ] = 0x02020202;      /* R2  */
    pstContext->auwR[3 ] = 0x03030303;      /* R3  */
    pstContext->auwR[4 ] = 0x04040404;      /* R4  */
    pstContext->auwR[5 ] = 0x05050505;      /* R5  */
    pstContext->auwR[6 ] = 0x06060606;      /* R6  */
    pstContext->auwR[7 ] = 0x07070707;      /* R7  */
    pstContext->auwR[8 ] = 0x08080808;      /* R8  */
    pstContext->auwR[9 ] = 0x09090909;      /* R9  */
    pstContext->auwR[10] = 0x10101010;      /* R10 */
    pstContext->auwR[11] = 0x11111111;      /* R11 */
    pstContext->auwR[12] = 0x12121212;      /* R12 */

#ifdef LOSCFG_INTERWORK_THUMB
    pstContext->uwPSR = PSR_MODE_SVC_THUMB;    /* CPSR  (Enable IRQ and FIQ interrupts, THUMNB-mode)    */
#else
    pstContext->uwPSR = PSR_MODE_SVC_ARM;    /* CPSR  (Enable IRQ and FIQ interrupts, ARM-mode)    */
#endif


    pstContext->D[0 ] = 0xAAA0000000000000LL;      /* D0  */
    pstContext->D[1 ] = 0xAAA0000000000001LL;      /* D1  */
    pstContext->D[2 ] = 0xAAA0000000000002LL;      /* D2  */
    pstContext->D[3 ] = 0xAAA0000000000003LL;      /* D3  */
    pstContext->D[4 ] = 0xAAA0000000000004LL;      /* D4  */
    pstContext->D[5 ] = 0xAAA0000000000005LL;      /* D5  */
    pstContext->D[6 ] = 0xAAA0000000000006LL;      /* D6  */
    pstContext->D[7 ] = 0xAAA0000000000007LL;      /* D7  */
    pstContext->D[8 ] = 0xAAA0000000000008LL;      /* D8  */
    pstContext->D[9 ] = 0xAAA0000000000009LL;      /* D9  */
    pstContext->D[10] = 0xAAA000000000000ALL;      /* D10 */
    pstContext->D[11] = 0xAAA000000000000BLL;      /* D11 */
    pstContext->D[12] = 0xAAA000000000000CLL;      /* D12 */
    pstContext->D[13] = 0xAAA000000000000DLL;      /* D13 */
    pstContext->D[14] = 0xAAA000000000000ELL;      /* D14 */
    pstContext->D[15] = 0xAAA000000000000FLL;      /* D15 */
    pstContext->D[16] = 0xAAA0000000000010LL;      /* D16 */
    pstContext->D[17] = 0xAAA0000000000011LL;      /* D17 */
    pstContext->D[18] = 0xAAA0000000000012LL;      /* D18 */
    pstContext->D[19] = 0xAAA0000000000013LL;      /* D19 */
    pstContext->D[20] = 0xAAA0000000000014LL;      /* D20 */
    pstContext->D[21] = 0xAAA0000000000015LL;      /* D21 */
    pstContext->D[22] = 0xAAA0000000000016LL;      /* D22 */
    pstContext->D[23] = 0xAAA0000000000017LL;      /* D23 */
    pstContext->D[24] = 0xAAA0000000000018LL;      /* D24 */
    pstContext->D[25] = 0xAAA0000000000019LL;      /* D25 */
    pstContext->D[26] = 0xAAA000000000001ALL;      /* D26 */
    pstContext->D[27] = 0xAAA000000000001BLL;      /* D27 */
    pstContext->D[28] = 0xAAA000000000001CLL;      /* D28 */
    pstContext->D[29] = 0xAAA000000000001DLL;      /* D29 */
    pstContext->D[30] = 0xAAA000000000001ELL;      /* D30 */
    pstContext->D[31] = 0xAAA000000000001FLL;      /* D31 */

    return (VOID *)pstContext;
}

void sev(void)
{
    __asm__ __volatile__ ("sev" : : : "memory");
}

void wfe(void)
{
    __asm__ __volatile__ ("wfe" : : : "memory");
}

void wfi(void)
{
    __asm__ __volatile__ ("wfi" : : : "memory");
}

void dmb(void)
{
     __asm__ __volatile__ ("dmb" : : : "memory");
}

void dsb(void)
{
    __asm__ __volatile__("dsb" : : : "memory");
}

void isb(void)
{
    __asm__ __volatile__("isb" : : : "memory");
}

void flush_icache(void)
{
    __asm__ __volatile__ ("mcr p15, 0, %0, c7, c5, 0" : : "r" (0) : "memory");
}

void flush_dcache(int start, int end)
{
    v7_dma_clean_range(start, end);
}


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
