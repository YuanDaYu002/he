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

#include "los_hwi.h"
#include "los_memory.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

UINT32  g_vuwIntCount = 0;
HWI_HANDLE_FORM_S m_astHwiForm[OS_HWI_MAX_NUM];
UINT32 g_vuwHwiFormCnt[OS_HWI_MAX_NUM];

/*****************************************************************************
 Function    : osHwiInit
 Description : initialization of the hardware interrupt
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
VOID osHwiInit(void)
{
    UINT32 uwHwiNum;

    for (uwHwiNum = 0; uwHwiNum < OS_HWI_MAX_NUM; uwHwiNum++)
    {
        m_astHwiForm[uwHwiNum].pfnHook = (HWI_PROC_FUNC)NULL; /*lint !e611*/
        m_astHwiForm[uwHwiNum].uwParam = 0;
        m_astHwiForm[uwHwiNum].pstNext = (struct tagHwiHandleForm *)NULL;
    }

    hal_interrupt_init();

    return ;
}

/*****************************************************************************
 Function    : LOS_HwiCreate
 Description : create hardware interrupt
 Input       : uwHwiNum   --- hwi num to create
               pfnHandler --- hwi handler
               usHwiPrio  --- priority of the hwi
               uwArg      --- param of the hwi handler
 Output      : None
 Return      : OS_SUCCESS on success or error code on failure
 *****************************************************************************/
UINT32 LOS_HwiCreate( HWI_HANDLE_T  uwHwiNum,
                                       HWI_PRIOR_T   usHwiPrio,
                                       HWI_MODE_T    usMode,
                                      HWI_PROC_FUNC pfnHandler,
                                       HWI_ARG_T     uwArg
                                      )
{
    UINTPTR uvIntSave;

#ifndef LOSCFG_NO_SHARED_IRQ
    HWI_HANDLE_FORM_S *pstHwiForm;
    HWI_HANDLE_FORM_S * pstHwiFormNode;
#endif
    if (NULL == pfnHandler)
    {
        return OS_ERRNO_HWI_PROC_FUNC_NULL;
    }

    if (uwHwiNum > OS_USER_HWI_MAX || uwHwiNum < OS_USER_HWI_MIN) /*lint !e568 !e685*/
    {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    uvIntSave = LOS_IntLock();

#ifdef LOSCFG_NO_SHARED_IRQ
    if(m_astHwiForm[uwHwiNum].pfnHook == (HWI_PROC_FUNC)NULL){
        m_astHwiForm[uwHwiNum].pfnHook = pfnHandler;
        m_astHwiForm[uwHwiNum].uwParam = uwArg;
    }
    else
        goto HANDLERONLIST;
#else
    pstHwiForm = &m_astHwiForm[uwHwiNum];

    while(NULL != pstHwiForm->pstNext)
    {
        pstHwiForm = pstHwiForm->pstNext;
        if(((tagIrqParam *)(pstHwiForm->uwParam) == (tagIrqParam *)(uwArg)) ||
            ((NULL != (tagIrqParam *)(uwArg)) &&(NULL != (tagIrqParam *)(pstHwiForm->uwParam)) && (((tagIrqParam *)(pstHwiForm->uwParam))->pDevId == ((tagIrqParam *)uwArg)->pDevId)) )/*lint !e413*/
            goto HANDLERONLIST;
    }
    pstHwiFormNode = (HWI_HANDLE_FORM_S *)LOS_MemAlloc( m_aucSysMem0, sizeof(HWI_HANDLE_FORM_S ));
    if(NULL == pstHwiFormNode)
    {
        LOS_IntRestore(uvIntSave);
        return OS_ERRNO_HWI_NO_MEMORY;
    }
    pstHwiFormNode->pfnHook = pfnHandler;
    pstHwiFormNode->uwParam = uwArg;
    pstHwiFormNode->pstNext = (struct tagHwiHandleForm *)NULL;
    pstHwiForm->pstNext = pstHwiFormNode;
#endif
    LOS_IntRestore(uvIntSave);
    return LOS_OK;

HANDLERONLIST:
    LOS_IntRestore(uvIntSave);
    return OS_ERRNO_HWI_ALREADY_CREATED;
}




#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


