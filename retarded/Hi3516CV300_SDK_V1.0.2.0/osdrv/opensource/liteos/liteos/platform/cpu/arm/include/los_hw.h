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
/** @defgroup los_hw Hardware
 *  @ingroup kernel
*/
#ifndef _LOS_HW_H
#define _LOS_HW_H

#include "los_base.h"
#include "asm/dma.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define PSR_T_ARM                                           0x00000000u
#define PSR_T_THUMB                                         0x00000020u
#define PSR_MODE_SVC                                        0x00000013u
#define PSR_MODE_SYS                                        0x0000001Fu

#define PSR_MODE_SVC_THUMB                                  (PSR_MODE_SVC | PSR_T_THUMB)
#define PSR_MODE_SVC_ARM                                    (PSR_MODE_SVC | PSR_T_ARM)

#define PSR_MODE_SYS_THUMB                                  (PSR_MODE_SYS | PSR_T_THUMB)
#define PSR_MODE_SYS_ARM                                    (PSR_MODE_SYS | PSR_T_ARM)

#define LOS_CHECK_SCHEDULE       ((!OS_INT_ACTIVE) && (!g_usLosTaskLock))

/**
 *@ingroup los_hw
 *@brief Trigger one task schedule.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to trigger one task schedule.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>This API won't be effective when it's called in interrupt or in the running task which is locked </li>
 *</ul>
 *
 *@param None.
 *
 *@retval #None.
 *
 *@par Dependency:
 *los_hw.h: the header file that contains the API declaration.
 *@see None.
 *@since Huawei LiteOS V100R001C00
*/
extern VOID LOS_Schedule(VOID);

/**
 *@ingroup los_hw
 *@brief Set Event.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to send an event to all cores within a muti-processor system.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>This API is not implemented.</li>
 *</ul>
 *
 *@param None.
 *
 *@retval #None.
 *
 *@par Dependency:
 *los_hw.h: the header file that contains the API declaration.
 *@see wfe.
 *@since Huawei LiteOS V100R001C00
*/
extern void sev(void);

/**
 *@ingroup los_hw
 *@brief Wait for event.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to suspend execution until events occurs if the event register is not set.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>This API is not implemented.</li>
 *</ul>
 *
 *@param None.
 *
 *@retval #None.
 *
 *@par Dependency:
 *los_hw.h: the header file that contains the API declaration.
 *@see sev.
 *@since Huawei LiteOS V100R001C00
*/
extern void wfe(void);

/**
 *@ingroup los_hw
 *@brief Wait for interrupt.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to suspend execution until interrupt or a debug request occurs.</li>
 *</ul>
 *@attention None.
 *
 *@param None.
 *
 *@retval #None.
 *
 *@par Dependency:
 *los_hw.h: the header file that contains the API declaration.
 *@see None.
 *@since Huawei LiteOS V100R001C00
*/
extern void wfi(void);

/**
 *@ingroup los_hw
 *@brief Data Memory Barrier.
 *
 *@par Description:
 *<ul>
 *<li>This API is used as a memory barrier</li>
 *</ul>
 *@attention None.
 *
 *@param None.
 *
 *@retval #None.
 *
 *@par Dependency:
 *los_hw.h: the header file that contains the API declaration.
 *@see None.
 *@since Huawei LiteOS V100R001C00
*/
extern void dmb(void);

/**
 *@ingroup los_hw
 *@brief Data Synchronization Barrier.
 *
 *@par Description:
 *<ul>
 *<li>This API is used as a special kind of memory barrier</li>
 *</ul>
 *@attention None.
 *
 *@param None.
 *
 *@retval #None.
 *
 *@par Dependency:
 *los_hw.h: the header file that contains the API declaration.
 *@see None.
 *@since Huawei LiteOS V100R001C00
*/
extern void dsb(void);

/**
 *@ingroup los_hw
 *@brief Instruction Synchronization Barrier.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to flush the pipeline in the processor, so that all instructions following the ISB are fetched from cache or memory, after the instruction has been completed.</li>
 *</ul>
 *@attention None.
 *
 *@param None.
 *
 *@retval #None.
 *
 *@par Dependency:
 *los_hw.h: the header file that contains the API declaration.
 *@see None.
 *@since Huawei LiteOS V100R001C00
*/
extern void isb(void);

/**
 *@ingroup los_hw
 *@brief Flush instruction cache.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to flush the instruction cache to the memory.</li>
 *</ul>
 *@attention None.
 *
 *@param None.
 *
 *@retval #None.
 *
 *@par Dependency:
 *los_hw.h: the header file that contains the API declaration.
 *@see None.
 *@since Huawei LiteOS V100R001C00
*/
extern void flush_icache(void);

/**
 *@ingroup los_hw
 *@brief Flush data cache.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to flush the data cache to the memory.</li>
 *</ul>
 *@attention
 *<ul>
 *<li>The input end address must be greater than the input start address.</li>
 *</ul>
 *
 *@param  start [IN] Type #int Flush start address.
 *@param  end [IN] Type #int Flush end address.
 *
 *@retval #None.
 *
 *@par Dependency:
 *los_hw.h: the header file that contains the API declaration.
 *@see None.
 *@since Huawei LiteOS V100R001C00
*/
extern void flush_dcache(int start, int end);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


#endif /* _LOS_HW_H */

