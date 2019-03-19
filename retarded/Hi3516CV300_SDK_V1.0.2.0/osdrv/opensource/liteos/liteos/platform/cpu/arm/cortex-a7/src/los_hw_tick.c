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

#include "los_hw.ph"
#include "los_tick.ph"

#include "los_hwi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

extern UINT32 g_uwTicksPerSec;
extern UINT64      g_ullTickCount;
extern UINT32      g_uwCyclePerSec;
extern UINT32 osTickTmrCreate(UINT32 uwSysClk ,UINT32 uwTicksPerSec);
extern void hal_clock_initialize(UINT32 period);
UINT32 osTickInit(UINT32 uwSystemClock, UINT32 uwTickPerSecond)
{

    //uwSystemClock = uwSystemClock / 255 ; //50M/256 frequency division
    if ((0 == uwSystemClock)
        || (0 == uwTickPerSecond)
        || (uwTickPerSecond > uwSystemClock))
    {
        return LOS_ERRNO_TICK_CFG_INVALID;
    }
    g_uwTicksPerSec = uwTickPerSecond;
    g_uwCyclePerSec = uwSystemClock;

    g_ullTickCount = 0;
    hal_clock_initialize(uwSystemClock / uwTickPerSecond);

    return LOS_OK;
    //return osTickTmrCreate(uwSystemClock ,uwTickPerSecond);
}
extern void hal_clock_enable(void);
extern VOID tick_interrupt_unmask(VOID);
VOID osTickStart(VOID)
{
    hal_clock_enable();
    tick_interrupt_unmask();
    //OS_TMR0_BASE_REG->uwTCR   = OS_REG_TCR_EN;      /* Enable timer0                          */
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


