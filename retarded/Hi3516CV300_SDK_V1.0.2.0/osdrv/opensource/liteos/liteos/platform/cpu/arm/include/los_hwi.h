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

 /**@defgroup los_hwi Hardware interrupt
   *@ingroup kernel
 */
#ifndef _LOS_HWI_H
#define _LOS_HWI_H

#include "los_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


/**
 * @ingroup los_hwi
 *Count of interrupts.
 */
extern UINT32  g_vuwIntCount;
/**
 * @ingroup los_hwi
 *An interrupt is active.
 */
#define OS_INT_ACTIVE    (g_vuwIntCount > 0)
/**
 * @ingroup los_hwi
 *An interrupt is inactive.
 */
#define OS_INT_INACTIVE                                     	(!(OS_INT_ACTIVE))

/**
 * @ingroup los_hwi
 * Highest priority of a hardware interrupt.
 */
#define  OS_HWI_PRIO_HIGHEST                          0

/**
 * @ingroup los_hwi
 * Lowest priority of a hardware interrupt.
 */
#define  OS_HWI_PRIO_LOWEST                           31


/**
 * @ingroup los_hwi
 * Hardware interrupt error code: Invalid interrupt number.
 *
 * Value: 0x02000900
 *
 * Solution: Ensure that the interrupt number is valid. The value range of the interrupt number applicable for an ARM926 platform is [0,31].
 */
#define OS_ERRNO_HWI_NUM_INVALID                            LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x00)

/**
 * @ingroup los_hwi
 * Hardware interrupt error code: Null hardware interrupt handling function.
 *
 * Value: 0x02000901
 *
 * Solution: Pass in a valid non-null hardware interrupt handling function.
 */
#define OS_ERRNO_HWI_PROC_FUNC_NULL                         LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x01)

/**
 * @ingroup los_hwi
 * Hardware interrupt error code: Insufficient interrupt resources for hardware interrupt creation.
 *
 * Value: 0x02000902
 *
 * Solution: Increase the configured maximum number of supported hardware interrupts.
 */
#define OS_ERRNO_HWI_CB_UNAVAILABLE                         LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x02)


/**
 * @ingroup los_hwi
 * Hardware interrupt error code: Insufficient memory for hardware interrupt initialization.
 *
 * Value: 0x02000903
 *
 * Solution: Expand the configured memory.
 */
#define OS_ERRNO_HWI_NO_MEMORY                              LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x03)

/**
 * @ingroup los_hwi
 * Hardware interrupt error code: The interrupt has already been created.
 *
 * Value: 0x02000904
 *
 * Solution: Check whether the interrupt specified by the passed-in interrupt number has already been created.
 */
#define OS_ERRNO_HWI_ALREADY_CREATED                        LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x04)

/**
 * @ingroup los_hwi
 * Hardware interrupt error code: Invalid interrupt priority.
 *
 * Value: 0x02000905
 *
 * Solution: Ensure that the interrupt priority is valid. The value range of the interrupt priority applicable for an ARM926 platform is [0,31].
 */
#define OS_ERRNO_HWI_PRIO_INVALID                           LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x05)

/**
 * @ingroup los_hwi
 * Hardware interrupt error code: Incorrect interrupt creation mode.
 *
 * Value: 0x02000906
 *
 * Solution: The interrupt creation mode can be only set to OS_HWI_MODE_COMM or OS_HWI_MODE_FAST of which the value can be 0 or 1.
 */
#define OS_ERRNO_HWI_MODE_INVALID                           LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x06)

/**
 * @ingroup los_hwi
 * Hardware interrupt error code: The interrupt has already been created as a fast interrupt.
 *
 * Value: 0x02000907
 *
 * Solution: Check whether the interrupt specified by the passed-in interrupt number has already been created.
 */
#define OS_ERRNO_HWI_FASTMODE_ALREADY_CREATED               LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x07)

/**
 * @ingroup los_hwi
 * Hardware interrupt error code: The API is called during an interrupt, which is forbidden.
 *
 * Value: 0x02000908
 *
 * * Solution: Do not call the API during an interrupt.
 */
#define OS_ERRNO_HWI_INTERR               LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x08)


/**
 * @ingroup los_hwi
 * Define the type of a hardware interrupt number.
 */
typedef UINT32                                              HWI_HANDLE_T;

/**
 * @ingroup los_hwi
 * Define the type of a hardware interrupt priority.
 */
typedef UINT16                                              HWI_PRIOR_T;

/**
 * @ingroup los_hwi
 * Define the type of hardware interrupt mode configurations.
 */
typedef UINT16                                              HWI_MODE_T;

/**
 * @ingroup los_hwi
 * Define the type of the parameter used for the hardware interrupt creation function. The function of this parameter varies among platforms.
 */
typedef UINT32                                              HWI_ARG_T;

/**
 * @ingroup  los_hwi
 * Define the type of a hardware interrupt handling function.
 */
typedef VOID (* HWI_PROC_FUNC)(void);



typedef struct tagHwiHandleForm
{
    HWI_PROC_FUNC pfnHook;
    HWI_ARG_T     uwParam;
    struct tagHwiHandleForm *pstNext;
    UINT32 uwreserved;
} HWI_HANDLE_FORM_S;

typedef struct tagIrqParam
{
    int             swIrq;
    void          *pDevId;
}tagIrqParam;

extern HWI_HANDLE_FORM_S m_astHwiForm[OS_HWI_MAX_NUM];

/**
 *@ingroup los_hwi
 *@brief Disable all interrupts.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to disable all IRQ and FIQ interrupts in the CPSR.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param None.
 *
 *@retval #UINTPTR CPSR value obtained before all interrupts are disabled.
 *@par Dependency:
 *<ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_IntRestore
 *@since Huawei LiteOS V100R001C00
 */
#ifdef LOSCFG_ARM926
#ifdef LOSCFG_INTERWORK_THUMB
extern	 UINTPTR LOS_IntLock(VOID);
#else
STATIC INLINE UINTPTR LOS_IntLock(VOID)
{
      UINT32 uwRet = 0,temp;
    __asm__ __volatile__(
        "MRS %0,cpsr\n"
        "ORR %1,%0,#0xc0\n"
        "MSR cpsr_c,%1"
        :"=r"(uwRet),  "=r"(temp)
        :
        :"memory");
    return uwRet;
} /*lint !e529*/
#endif

#elif defined(LOSCFG_CORTEX_A7) || defined(LOSCFG_CORTEX_A17)
STATIC INLINE UINTPTR LOS_IntLock(VOID)
{
    UINTPTR uvIntSave = 0;

    __asm__ __volatile__(
        "mrs %0, cpsr"
        : "=r"(uvIntSave));

    __asm__ __volatile__("cpsid if");


    return uvIntSave;
}
#endif

/**
 *@ingroup los_hwi
 *@brief Enable all interrupts.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to enable all IRQ and FIQ interrupts in the CPSR.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>None.</li>
 *</ul>
 *
 *@param None.
 *
 *@retval #UINTPTR CPSR value obtained after all interrupts are enabled.
 *@par Dependency:
 *<ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_IntLock
 *@since Huawei LiteOS V100R001C00
 */
#ifdef LOSCFG_ARM926
#ifdef LOSCFG_INTERWORK_THUMB
extern	 UINTPTR LOS_IntUnLock(VOID);
#else
STATIC INLINE UINTPTR LOS_IntUnLock(VOID)
{
    UINTPTR uwCpsSave;
    __asm__ __volatile__(
        "MRS   %0, cpsr\n"
        "BIC   %0, %0, #0xc0\n"
        "MSR   cpsr_c, %0"
        :"=r"(uwCpsSave) ::);
    return uwCpsSave;

}
#endif
#elif defined(LOSCFG_CORTEX_A7) || defined(LOSCFG_CORTEX_A17)
STATIC INLINE UINTPTR LOS_IntUnLock(VOID)
{
    UINTPTR uwCpsSave = 0;
    __asm__ __volatile__(
        "MRS   %0, cpsr\n"
        "BIC   %0, %0, #0xc0\n"
        "MSR   cpsr_c, %0"
        :"=r"(uwCpsSave) ::);
    return uwCpsSave;
}
#endif

/**
 *@ingroup los_hwi
 *@brief Restore interrupts.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to restore the CPSR value obtained before all interrupts are disabled.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>This API can be called only after all interrupts are disabled, and the input parameter value should be the value returned by LOS_IntLock.</li>
 *</ul>
 *
 *@param uvIntSave [IN] Type #UINTPTR : CPSR value obtained before all interrupts are disabled.
 *
 *@retval None.
 *@par Dependency:
 *<ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 *@see LOS_IntLock
 *@since Huawei LiteOS V100R001C00
 */
#ifdef LOSCFG_ARM926
#ifdef LOSCFG_INTERWORK_THUMB
extern	 VOID LOS_IntRestore(UINTPTR uvIntSave);
#else
STATIC INLINE VOID LOS_IntRestore(UINTPTR uvIntSave)
{
    __asm__ __volatile__(
        "MSR     cpsr_c, %0"
        ::"r"(uvIntSave) );
}
#endif
#elif defined(LOSCFG_CORTEX_A7) || defined(LOSCFG_CORTEX_A17)
STATIC INLINE VOID LOS_IntRestore(UINTPTR uvIntSave)
{
    __asm__ __volatile__(
        "MSR     cpsr_c, %0"
        ::"r"(uvIntSave) );
}
#endif

/**
 * @ingroup  los_hwi
 * @brief Create a hardware interrupt.
 *
 * @par Description:
 * This API is used to configure a hardware interrupt and register a hardware interrupt handling function.
 *
 * @attention
 * <ul>
 * <li>The hardware interrupt module is usable only when the configuration item for hardware interrupt tailoring is enabled.</li>
 * <li>Hardware interrupt number value range: [OS_USER_HWI_MIN,OS_USER_HWI_MAX]. The value range applicable for an ARM926 platform is [0,31].</li>
 * <li>OS_HWI_MAX_NUM specifies the maximum number of interrupts that can be created.</li>
 * <li>Before executing an interrupt on a platform, refer to the chip manual of the platform.</li>
 * <li>The parameter pfnHandler of this interface is a interrupt handler, it should be correct, otherwise, the system may be abnormal.</li>
 * <li>The input uwArg could be 0, if not, it should be address which point to a struct tagIrqParam</li>
 * </ul>
 *
 * @param  uwHwiNum   [IN] Type #HWI_HANDLE_T: hardware interrupt number. The value range applicable for an ARM926 platform is [0,31].
 * @param  usHwiPrio  [IN] Type #HWI_PRIOR_T: hardware interrupt priority. Ignore this parameter temporarily.
 * @param  usMode     [IN] Type #HWI_MODE_T: hardware interrupt mode. Ignore this parameter temporarily.
 * @param  pfnHandler [IN] Type #HWI_PROC_FUNC: interrupt handler used when a hardware interrupt is triggered.
 * @param  uwArg      [IN] Type #HWI_ARG_T: input parameter of the interrupt handler used when a hardware interrupt is triggered.
 *
 * @retval #OS_ERRNO_HWI_PROC_FUNC_NULL              Null hardware interrupt handling function.
 * @retval #OS_ERRNO_HWI_NUM_INVALID                 Invalid interrupt number.
 * @retval #OS_ERRNO_HWI_NO_MEMORY                   Insufficient memory for hardware interrupt creation.
 * @retval #OS_ERRNO_HWI_ALREADY_CREATED             The interrupt handler being created has already been created.
 * @retval #LOS_OK                                   The interrupt is successfully created.
 * @par Dependency:
 * <ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 * @since Huawei LiteOS V100R001C00
 */
extern UINT32 LOS_HwiCreate( HWI_HANDLE_T  uwHwiNum,
                           HWI_PRIOR_T   usHwiPrio,
                           HWI_MODE_T    usMode,
                           HWI_PROC_FUNC pfnHandler,
                           HWI_ARG_T     uwArg
                           );


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


#endif /* _LOS_HWI_H */
