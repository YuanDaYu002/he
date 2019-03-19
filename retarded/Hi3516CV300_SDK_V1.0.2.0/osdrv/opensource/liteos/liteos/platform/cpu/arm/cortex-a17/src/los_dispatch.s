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
    .extern   g_stLosTask
    .extern   g_vuwIntCount
    .extern   g_usUniTaskLock
    .extern   osExceptSchedule
    @EXTERN   m_pfnTskSwitchHook
    @EXTERN   g_pfnHwiEntryHook
    @EXTERN   g_pfnHwiExitHook
    @EXTERN   ||Image$$ARM_LIB_STACKHEAP$$ZI$$Limit||

    .global   _osLoadNewTask
    .global   osTaskSchedule
    .global   _TaskSwitch
    .global   _osExceptIrqHdl
    .global   osStartToRun
    .global  osIrqDisable
    .global  osIrqEnable

    .extern  OSRunning
    .extern  OSPrioCur
    .extern  OSPrioHighRdy
    .extern  OSTCBCur
    .extern  OSTCBHighRdy
    .extern  OSIntNesting
    .extern  OSIntExit
    .extern  OSTaskSwHook
    .extern  _TaskSwitch
    .extern  m_astHwiForm
    .extern __svc_stack_top
    .extern __irq_stack_top
    .extern osTaskSwitchCheck
    .extern hal_IRQ_handler
    .extern g_curirqnum
    .extern hal_interrupt_acknowledge

@********************************************************************************************************
@                                                EQUATES
@********************************************************************************************************

.equ OS_PSR_THUMB, 0x20                @ Thumb instruction set
.equ OS_PSR_INT_DIS, 0xC0                @ Disable both FIQ and IRQ
.equ OS_PSR_FIQ_DIS, 0x40                @ Disable FIQ
.equ OS_PSR_IRQ_DIS, 0x80                @ Disable IRQ
.equ OS_PSR_MODE_MASK, 0x1F
.equ OS_PSR_MODE_USR, 0x10
.equ OS_PSR_MODE_FIQ, 0x11
.equ OS_PSR_MODE_IRQ, 0x12
.equ OS_PSR_MODE_SVC, 0x13
.equ OS_PSR_MODE_ABT, 0x17
.equ OS_PSR_MODE_UND, 0x1B
.equ OS_PSR_MODE_SYS, 0x1F
.equ OS_REG_A7_PERI_GIC_DIST ,0x1000
.equ OS_REG_A7_PERI_GIC_CPU ,0x2000
.equ OS_REG_BASE_A7_PERI ,0x20300000
.equ OS_CFG_GIC_CPU_BASE ,0x20302000 @(REG_BASE_A7_PERI + REG_A7_PERI_GIC_CPU)
.equ OS_CFG_GIC_DIST_BASE ,0x20301000 @(REG_BASE_A7_PERI + REG_A7_PERI_GIC_DIST)

@ Define Vector Addr
@.equ OS_VECTOR_ADDR, 0xFFFFFF00

.equ TASK_STATUS_RUNNING, 0x0010
.equ OS_FLG_HWI_ACTIVE, 0x0001
.equ OS_ARM_PSR_THUMB, 0x20

.fpu vfpv4
@.fpu neon
.arch armv7a
osStartToRun:

    MSR    CPSR_c, #(OS_PSR_INT_DIS | OS_PSR_FIQ_DIS | OS_PSR_MODE_SVC)

    LDR     R0, =g_bTaskScheduled
    MOV     R1, #1
    STR     R1, [R0]

    @ g_stLosTask.pstRunTask = g_stLosTask.pstNewTask, if g_stLosTask.pstRunTask is  NULL, will fall to error
    LDR     R0, =g_stLosTask
    LDR     R0, [R0, #4]
    LDR     R1, =g_stLosTask
    STR     R0, [R1]

    VPUSH   {S0}                                     @fpu
    VPOP    {S0}
    VPUSH   {D0}
    VPOP    {D0}

    B       osTaskContextSwitch

osTaskSchedule:
    STMFD  SP!, {LR}  @ PC
    STMFD  SP!, {LR}  @ LR
    STMFD  SP!, {R0-R12}
    MRS    R0, CPSR
    CPSID  i
    TST    LR, #1
    ORRNE  R0, R0, #OS_ARM_PSR_THUMB
    STMFD  SP!, {R0}
    VSTMDB  SP!, {D16-D31}                                     @fpu
    VSTMDB  SP!, {D0-D15}                                     @fpu

    LDR    R0, =g_stLosTask
    LDR    R1, [R0]
    STR    SP, [R1]



    B      osTaskContextSwitch

@********************************************************************************************************
@                                      CODE GENERATION DIRECTIVES
@********************************************************************************************************

    .text

@********************************************************************************************************
@ Description: Interrupt request exception handler
@ Parameter  : None
@********************************************************************************************************
_osExceptIrqHdl:
    LDR     SP, =__irq_stack_top
    SUB     LR, LR, #4
    STMFD   SP!, {R0-R2}
    MOV     R0, SP
    MRS     R1, SPSR
    MOV     R2, LR

    MSR    CPSR_c, #(OS_PSR_INT_DIS | OS_PSR_FIQ_DIS | OS_PSR_MODE_SVC)
    STMFD   SP!, {R2}         @ Push task's PC,
    STMFD   SP!, {LR}         @ Push task's LR,
    STMFD   SP!, {R3-R12}     @ Push task's R12-R3,
    LDMFD   R0!, {R5-R7}      @ Move task's R2-R0 from exception stack to task's stack.
    STMFD   SP!, {R5-R7}
    STMFD   SP!, {R1}         @ Push task's CPSR (i.e. exception SPSR).
    VSTMDB  SP!, {D16-D31}
    VSTMDB  SP!, {D0-D15}

    LDR    R0, =g_vuwIntCount
    LDR    R1, [R0]
    CMP    R1, #0
    BNE    osHwiDispatchBegin

    @ g_pRunningTask->pStackPointer = SP
    LDR    R2, =g_stLosTask
    LDR    R3, [R2]
    STR    SP, [R3]

    LDR    SP, =__svc_stack_top


osHwiDispatchBegin:

    @ g_vuwIntCount++
    LDR    R0, =g_vuwIntCount
    LDR    R1, [R0]
    ADD    R1, #1
    STR    R1, [R0]

    @MRC    p15, 4, r4, c15, c0, 0  @ Read periph base address
    @ADD    R4, R4, #0x100
    @LDR    R4, =OS_CFG_GIC_CPU_BASE
    @LDR    R5, [R4, #0xc]
    @SWAP32 R5, R5
    @LDR    R1, =0x3ff         @get the ackid [9:0]
    @AND    R0, R5, R1

    BLX hal_IRQ_handler

    @CMP    R0, R1       @fix bug r5 value equal 0x3ff when open wifi
    @BEQ    osInterruptExit

    LDR    R0, =g_curirqnum
    LDR    R0, [R0]

    LDR    R1, =g_cmpirqnum
    LDR    R1, [R1]
    CMP    R1, #0
    BEQ    osInterruptExit

    @MOV    R7, R0			  @save irq number
    LSL     R0, #4             @r0*16 index the function
    LDR     R1, =m_astHwiForm
    ADD     R1, R1, r0
    LDR     R1, [R1,#8]
    CMP    R1, #0
    BEQ    osInterruptClear
Traversal:
    LDR    R12, [R1]          @ @r12 =@m_astHwiForm[index](->pstNext..)->pfnHook
    LDR    R0,  [R1, #4]       @ r0 = @m_astHwiForm[index](->pstNext..)->uwIntParam
    LDR    R2,  [R1,#8]      @R2 =@m_astHwiForm[index](->pstNext..)->pstNext
    CMP    R0,  #0
    BEQ    osHwifnHandler
    MOV    R3, R0
    LDR    R0, [R3]
    LDR    R1, [R3, #4]

osHwifnHandler:
    STMFD  SP!, {R0-R12}
    SUB    SP, SP, #4 @sp requires eight-byte alignment of the stack
    BLX    R12
    ADD    SP, SP, #4
    LDMFD  SP!, {R0-R12}

    MOV    R1, R2
    CMP    R2, #0
    BNE    Traversal

osInterruptClear:
    @SWAP32 R5, R5
    @STR    R5, [R4, #0x10]    @clear fiq bit
    LDR    R0, =g_curirqnum
    LDR    R0, [R0]
    BLX    hal_interrupt_acknowledge

osInterruptExit:
    @ g_vuwIntCount--
    LDR   R0, =g_vuwIntCount
    LDR   R1, [R0]
    SUBS  R1, #1
    STR   R1, [R0]

    @ nest irq: g_vuwIntCount > 0
    BNE   osThrdContextLoad

osTaskContextSwitch:
    LDR     R0, =g_usLosTaskLock
    LDR     R0, [R0]
    LDR     R1, =g_stLosTask
    LDR     R1, [R1]
    CMP     R0, #0
    BNE    osTaskContextLoad

    LDR     R0, =g_stLosTask
    LDR     R0, [R0, #4]
    CMP    R1, R0
    BEQ    osTaskContextLoad


    @ g_pRunningTask->usTaskStatus &= ~OS_TSK_RUNNING
    LDRH   R7, [R1, #4]
    BIC    R7, #TASK_STATUS_RUNNING
    STRH   R7, [R1, #4]

    STMFD  SP!, {R0-R12}
    SUB    SP, SP, #4 @sp requires eight-byte alignment of the stack
    BLX    osTaskSwitchCheck
    ADD    SP, SP, #4
    LDMFD  SP!, {R0-R12}


    @ g_pRunningTask  = g_pHighestTask
    LDR     R0, =g_stLosTask
    LDR     R0, [R0, #4]
    LDR     R1, =g_stLosTask
    STR     R0, [R1]
    LDR     R1, [R1]

    @ g_pRunningTask->usTaskStatus |= OS_TSK_RUNNING
    LDRH   R7, [R0, #4] @LDRH
    ORR    R7, #TASK_STATUS_RUNNING
    STRH   R7, [R0, #4]


osTaskContextLoad:
    CLREX                                                   @clear the flag of ldrex
    @ SP = g_pRunningTask->pStackPointer
    LDR    SP, [R1]

osThrdContextLoad:
    VLDMIA  SP!, {D0-D15}                                     @fpu
    VLDMIA  SP!, {D16-D31}                                     @fpu
    LDMFD  SP!, {R0}
    @MRS    R0,  CPSR
    MSR    SPSR_cxsf, R0

    LDMFD  SP!, {R0-R12, LR, PC}^

    .end
