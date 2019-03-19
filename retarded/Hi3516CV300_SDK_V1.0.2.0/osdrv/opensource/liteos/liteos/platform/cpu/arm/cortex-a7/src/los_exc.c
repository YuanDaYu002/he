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
#include "los_exc.h"
#include "los_task.ph"
#include "los_printf.ph"
#include "los_hw.ph"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

VOID osExcHook(UINT32 uwExcType, EXC_CONTEXT_S *puwExcBufAddr);
VOID osExcStackInfo(VOID);
VOID BackTrace(UINT32 uwFP);

#define OS_MAX_BACKTRACE 15

EXC_PROC_FUNC g_pExcHook = (EXC_PROC_FUNC)osExcHook;
EXC_INFO_S m_stExcInfo;
UINT32 g_uwCurNestCount = 0;

static UINT32 osShellCmdTskInfoGetRef(UINT32 uwTaskID, UINT32 TransId) __attribute__((weakref("osShellCmdTskInfoGet")));  /*lint -e402*/

/*****************************************************************************
Function   : LOS_ExcRegHook
Description: register user hook function of EXC
Input   : pfnHook --- user hook function
Output  : None
Return  : None
Other   : None
*****************************************************************************/
UINT32 LOS_ExcRegHook( EXC_PROC_FUNC pfnHook )
{
    UINTPTR uvIntSave;
    uvIntSave = LOS_IntLock();
    g_pExcHook = pfnHook;

    LOS_IntRestore(uvIntSave);
    return LOS_OK;
}

VOID osExcHook(UINT32 uwExcType, EXC_CONTEXT_S *puwExcBufAddr)
{
    g_uwCurNestCount++;

    PRINTK("uwExcType = 0x%x\n", uwExcType);

    PRINTK("puwExcBuffAddr pc = 0x%x\n", puwExcBufAddr->uwPC);
    PRINTK("puwExcBuffAddr lr = 0x%x\n", puwExcBufAddr->uwLR);
    PRINTK("puwExcBuffAddr sp = 0x%x\n", puwExcBufAddr->uwSP);
    PRINTK("puwExcBuffAddr fp = 0x%x\n", puwExcBufAddr->uwR11);

    /*undefinited exception handling*/
    if (uwExcType == OS_EXCEPT_UNDEF_INSTR)
    {
        PRINTK("step into undef\n");

        if (((puwExcBufAddr->uwCPSR) & 0x01000020) == 0) /*work status: ARM*/
        {
            puwExcBufAddr->uwPC = puwExcBufAddr->uwPC - 4;
        }
        else if (((puwExcBufAddr->uwCPSR) & 0x01000020) == 0x20) /*work status: Thumb*/
        {
            puwExcBufAddr->uwPC = puwExcBufAddr->uwPC - 2;
        }
    }

    BackTrace(puwExcBufAddr->uwR11);

    PRINTK("R0         = 0x%x\n", puwExcBufAddr->uwR0);
    PRINTK("R1         = 0x%x\n", puwExcBufAddr->uwR1);
    PRINTK("R2         = 0x%x\n", puwExcBufAddr->uwR2);
    PRINTK("R3         = 0x%x\n", puwExcBufAddr->uwR3);
    PRINTK("R4         = 0x%x\n", puwExcBufAddr->uwR4);
    PRINTK("R5         = 0x%x\n", puwExcBufAddr->uwR5);
    PRINTK("R6         = 0x%x\n", puwExcBufAddr->uwR6);
    PRINTK("R7         = 0x%x\n", puwExcBufAddr->uwR7);
    PRINTK("R8         = 0x%x\n", puwExcBufAddr->uwR8);
    PRINTK("R9         = 0x%x\n", puwExcBufAddr->uwR9);
    PRINTK("R10        = 0x%x\n", puwExcBufAddr->uwR10);
    PRINTK("R11        = 0x%x\n", puwExcBufAddr->uwR11);
    PRINTK("R12        = 0x%x\n", puwExcBufAddr->uwR12);
    PRINTK("SP         = 0x%x\n", puwExcBufAddr->uwSP);
    PRINTK("LR         = 0x%x\n", puwExcBufAddr->uwLR);
    PRINTK("PC         = 0x%x\n", puwExcBufAddr->uwPC);
    PRINTK("CPSR       = 0x%x\n", puwExcBufAddr->uwCPSR);
    PRINTK("pcTaskName = %s\n", g_stLosTask.pstRunTask->pcTaskName);
    PRINTK("TaskID = %d\n", g_stLosTask.pstRunTask->uwTaskID);
    PRINTK("Task StackSize = %d\n", g_stLosTask.pstRunTask->uwStackSize);
    PRINTK("system mem addr:0x%x\n",m_aucSysMem0);
    osExcStackInfo();
}

extern UINT8 *m_aucSysMem0;

extern UINT32 __startup_stack_top;
extern UINT32 __irq_stack_top;
extern UINT32 __fiq_stack_top;
extern UINT32 __svc_stack_top;
extern UINT32 __abt_stack_top;
extern UINT32 __undef_stack_top;

void osExcStackInfo(void)
{
    UINT32 uwFigNum = 0, uwI = 0;
    UINT8 *p_t = (UINT8 *)&__undef_stack_top;
    PRINTK("\nstack name           stack addr     total size   used size\n"
               "----------           ---------      --------     --------\n");
    for (uwI = 0, uwFigNum =0 ;uwI<OS_UNDEF_STACK_SIZE; uwI++)
    {
        if (*(p_t-uwI) != 0)  /*lint !e676*///todo
            ++uwFigNum;
    }
    PRINTK("undef_stack_addr     0x%-10x   0x%-8x   0x%-4x\n",p_t, uwI, uwFigNum);

    p_t = (UINT8 *)&__abt_stack_top;
    for (uwI= 0, uwFigNum =0 ;uwI<OS_ABT_STACK_SIZE; uwI++)
    {
        if (*(p_t-uwI) != 0)  /*lint !e676*///todo
            ++uwFigNum;
    }
    PRINTK("abt_stack_addr       0x%-10x   0x%-8x   0x%-4x\n",p_t, uwI, uwFigNum);

    p_t = (UINT8 *)&__irq_stack_top;
    for (uwI = 0, uwFigNum =0 ;uwI<OS_IRQ_STACK_SIZE;uwI++)
    {
        if (*(p_t-uwI) != 0)  /*lint !e676*///todo
            ++uwFigNum;
    }
    PRINTK("irq_stack_addr       0x%-10x   0x%-8x   0x%-4x\n",p_t, uwI, uwFigNum);

    p_t = (UINT8 *)&__fiq_stack_top;
    for (uwI= 0, uwFigNum =0 ;uwI<OS_FIQ_STACK_SIZE;uwI++)
    {
        if (*(p_t-uwI) != 0)  /*lint !e676*///todo
            ++uwFigNum;
    }
    PRINTK("fiq_stack_addr       0x%-10x   0x%-8x   0x%-4x\n",p_t, uwI, uwFigNum);

    p_t = (UINT8 *)&__svc_stack_top;
    for (uwI= 0, uwFigNum =0 ;uwI<OS_SVC_STACK_SIZE;uwI++)
    {
        if (*(p_t-uwI) != 0)  /*lint !e676*///todo
             ++uwFigNum;
    }
    PRINTK("svc_stack_addr       0x%-10x   0x%-8x   0x%-4x\n",p_t, uwI, uwFigNum);

    p_t = (UINT8 *)&__startup_stack_top;
    for(uwI= 0, uwFigNum =0 ;uwI<OS_STARTUP_STACK_SIZE;uwI++)
    {
        if (*(p_t-uwI) != 0)  /*lint !e676*///todo
            ++uwFigNum;
    }
    PRINTK("startup_stack_addr   0x%-10x   0x%-8x   0x%-4x\n",p_t, uwI, uwFigNum);
}

__attribute__ ((noinline))void LOS_Panic(const char * fmt, ...)
{
    va_list ap;
    va_start(ap,fmt);
    _dprintf(fmt, ap);
    va_end(ap);
    __asm__ __volatile__("swi 0");
}/*lint !e438*/

void BackTrace(UINT32 uwFP)
{
    UINT32 uwBackFP = uwFP;
    UINT32 uwtmpFP;
    UINT32 uwBackLR;
    UINT32 uwCount = 0;
    PRINTK("*******backtrace begin*******\n");
    while( uwBackFP>OS_SYS_FUNC_ADDR_START && uwBackFP<OS_SYS_FUNC_ADDR_END )
    {
        uwtmpFP = uwBackFP;
        uwBackLR = *((UINT32 *)(uwtmpFP));
        uwBackFP = *((UINT32 *)(uwtmpFP-4));
        PRINTK("traceback %d -- lr = 0x%x\n", uwCount, uwBackLR);
        PRINTK("traceback %d -- fp = 0x%x\n", uwCount, uwBackFP);
        uwCount++;
        if(uwCount == OS_MAX_BACKTRACE)
            break;
    }
    if (osShellCmdTskInfoGetRef) /*lint !e506*/
    {
        (VOID)osShellCmdTskInfoGetRef(0xffffffff, 0);
    }
}

void osTaskBackTrace(UINT32 uwTaskID)
{
    LOS_TASK_CB *pstTaskCB;

    if(uwTaskID >= g_uwTskMaxNum)
    {
        PRINT_ERR("\r\nTask PID is invalid!\n");
        return;
    }
    pstTaskCB = OS_TCB_FROM_TID(uwTaskID);

    if((pstTaskCB->usTaskStatus & OS_TASK_STATUS_UNUSED) \
            || (pstTaskCB->pfnTaskEntry == NULL)\
            || (pstTaskCB->pcTaskName == NULL))
    {
        PRINT_ERR("\r\nThe task is not created!\n");
        return;
    }
    PRINTK("TaskName = %s\n", pstTaskCB->pcTaskName);
    PRINTK("TaskID = 0x%x\n", pstTaskCB->uwTaskID);
    BackTrace(((TSK_CONTEXT_S  *)(pstTaskCB->pStackPointer))->auwR[11]); //0x130 is the offset of R11 relate to SP
}

void osBackTrace(void)
{
    UINT32 uwFp = Get_Fp();
    PRINTK("osBackTrace fp = 0x%x\n",uwFp);
    PRINTK("g_stLosTask.pstRunTask->pcTaskName = %s\n", g_stLosTask.pstRunTask->pcTaskName);
    PRINTK("g_stLosTask.pstRunTask->uwTaskID = %d\n", g_stLosTask.pstRunTask->uwTaskID);
    BackTrace(uwFp);
}

/*****************************************************************************
 Function    : osExcHandleEntry
 Description : EXC handler entry
 Input       : puwExcBufAddr ---address of EXC buf
 Output      : None
 Return      : None
 *****************************************************************************/
VOID osExcHandleEntry(UINT32 uwExcType, EXC_CONTEXT_S *puwExcBufAddr)
{
    if(g_pExcHook != NULL)
    {
        g_pExcHook(uwExcType, puwExcBufAddr);
    }
    while(1)
    {
        ;
    }
}

/************stack protector**************/
unsigned long __stack_chk_guard = 0xd00a0dff;

void __stack_chk_fail(void)
{
    LOS_Panic("stack-protector: Kernel stack is corrupted in: %p\n",
                __builtin_return_address(0)); /*lint !e1055*/
    //__builtin_return_address is a builtin function, building in gcc
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

