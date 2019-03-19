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

#ifndef _LOS_CONFIG_H
#define _LOS_CONFIG_H

#include "los_typedef.h"
#include "hisoc/clock.h"
#include "board.h"
#include "los_tick.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @ingroup los_config
 * int stack start addr
 */
extern char  __int_stack_start;

/****************************** System clock module configuration ****************************/

/**
 * @ingroup los_config
 * System clock (unit: HZ)
 */
#define OS_SYS_CLOCK                           (get_bus_clk())//lint -e653

/**
 * @ingroup los_config
 * time timer clock (unit: HZ)
 */
#define OS_TIME_TIMER_CLOCK                     (get_bus_clk())

/**
* @ingroup los_config
* limit addr range when search for  'func local(frame pointer)' or 'func name'
*/
#define OS_SYS_FUNC_ADDR_START      (unsigned int)&__int_stack_start
#define OS_SYS_FUNC_ADDR_END        g_sys_mem_addr_end

/**
* @ingroup los_config
* .int stack size
*/
#define OS_UNDEF_STACK_SIZE     32
#define OS_ABT_STACK_SIZE   32
#define OS_IRQ_STACK_SIZE   64
#define OS_FIQ_STACK_SIZE   64
#define OS_SVC_STACK_SIZE   4096

/**
* @ingroup los_config
* .startup stack size
*/
#define OS_STARTUP_STACK_SIZE   512

/**
 * @ingroup los_config
 * Number of Ticks in one second
 */
#define LOSCFG_BASE_CORE_TICK_PER_SECOND                     100

/**
 * @ingroup los_config
 * External configuration item for timer tailoring
 */
#define LOSCFG_BASE_CORE_TICK_HW_TIME                    YES
/****************************** Hardware interrupt module configuration ******************************/

/**
 * @ingroup los_config
 * Configuration item for hardware interrupt tailoring
 */
#define LOSCFG_PLATFORM_HWI                         YES

/**
 * @ingroup los_config
 * Maximum number of used hardware interrupts, including Tick timer interrupts.
 */
#define LOSCFG_PLATFORM_HWI_LIMIT                    32

/****************************** Task module configuration ********************************/

/**
 * @ingroup los_config
 * Minimum stack size.
 *
 * 0x400 bytes, aligned on a boundary of 4.
 */
#define LOS_TASK_MIN_STACK_SIZE                     (ALIGN(0x400, 4))

/**
 * @ingroup los_config
 * Default task priority
 */
#define LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO                10

/**
 * @ingroup los_config
 * Maximum supported number of tasks except the idle task rather than the number of usable tasks
 */
#define LOSCFG_BASE_CORE_TSK_LIMIT                 256

/**
 * @ingroup los_config
 * Size of the idle task stack
 */
#define LOSCFG_BASE_CORE_TSK_IDLE_STACK_SIZE                 SIZE(0x800)

/**
 * @ingroup los_config
 * Default task stack size
 */
#define LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE              SIZE(0x6000)

/**
 * @ingroup los_config
 * Configuration item for task Robin tailoring
 */
#define LOSCFG_BASE_CORE_TIMESLICE                        YES

/**
 * @ingroup los_config
 * Longest execution time of tasks with the same priorities
 */
#define LOSCFG_BASE_CORE_TIMESLICE_TIMEOUT                        2

/**
 * @ingroup los_config
 * Configuration item for task (stack) monitoring module tailoring
 */
#define LOSCFG_BASE_CORE_TSK_MONITOR                          YES

/**
 * @ingroup los_config
 * Configuration item for task perf task filter hook
 */
#define OS_PERF_TSK_FILTER                      NO

/****************************** Semaphore module configuration ******************************/

/**
 * @ingroup los_config
 * Configuration item for semaphore module tailoring
 */
#define LOSCFG_BASE_IPC_SEM                         YES

/**
 * @ingroup los_config
 * Maximum supported number of semaphores
 */
#define LOSCFG_BASE_IPC_SEM_LIMIT                  1024

/****************************** mutex module configuration ******************************/

/**
 * @ingroup los_config
 * Configuration item for mutex module tailoring
 */
#define LOSCFG_BASE_IPC_MUX                         YES

/**
 * @ingroup los_config
 * Maximum supported number of mutexes
 */
#define LOSCFG_BASE_IPC_MUX_LIMIT                 1024
/****************************** Queue module configuration ********************************/

/**
 * @ingroup los_config
 * Configuration item for queue module tailoring
 */
#define LOSCFG_BASE_IPC_QUEUE                       YES

/**
 * @ingroup los_config
 * Maximum supported number of queues rather than the number of usable queues
 */
#define LOSCFG_BASE_IPC_QUEUE_LIMIT               1024
/****************************** Software timer module configuration **************************/
#if (LOSCFG_BASE_IPC_QUEUE == YES)

/**
 * @ingroup los_config
 * Configuration item for software timer module tailoring
 */
#define LOSCFG_BASE_CORE_SWTMR                       YES

/**
 * @ingroup los_config
 * Maximum supported number of software timers rather than the number of usable software timers
 */
#define LOSCFG_BASE_CORE_SWTMR_LIMIT               1024
/**
 * @ingroup los_config
 * Max number of software timers ID
 */
#define OS_SWTMR_MAX_TIMERID                         ((65535/LOSCFG_BASE_CORE_SWTMR_LIMIT) * LOSCFG_BASE_CORE_SWTMR_LIMIT)

/**
 * @ingroup los_config
 * Maximum size of a software timer queue
 */
#define OS_SWTMR_HANDLE_QUEUE_SIZE             (LOSCFG_BASE_CORE_SWTMR_LIMIT + 0)
#endif

/****************************** Memory module configuration **************************/

/**
 * @ingroup los_config
 * Starting address of the memory
 */
#define OS_SYS_MEM_ADDR                         &m_aucSysMem0[0]

/**
 * @ingroup los_config
 * Memory size
 */
#define OS_SYS_MEM_SIZE                         ((g_sys_mem_addr_end) - OS_SYS_NOCACHEMEM_SIZE - ((((unsigned long)&__bss_end) + (64 - 1)) & ~(64 - 1)))

/****************************** Memory module configuration **************************/

/**
 * @ingroup los_config
 * Size of unaligned memory
 */
#define OS_SYS_NOCACHEMEM_SIZE                    0x0
#if OS_SYS_NOCACHEMEM_SIZE

/**
 * @ingroup los_config
 * Starting address of the unaligned memory
 */
#define OS_SYS_NOCACHEMEM_ADDR                    &m_aucSysNoCacheMem0[0]
#endif


/****************************** fw Interface configuration **************************/

/**
 * @ingroup los_config
 * Configuration item for the monitoring of task communication
 */
#define LOSCFG_COMPAT_CMSIS_FW                         YES

/**
 * @ingroup los_config
 * Version number
 */
#define _T(x)                                   x
#define HW_LITEOS_SYSNAME                       "Huawei LiteOS"
#define HW_LITEOS_SEP                           " "
#define _V(v)                                   _T(HW_LITEOS_SYSNAME)_T(HW_LITEOS_SEP)_T(v)

#define HW_LITEOS_VERSION                       "V200R001C10B035"
#define VER                                     _V(HW_LITEOS_VERSION)

/**
 * @ingroup los_config
 * The Version number of Public
 */
#define HW_LITEOS_OPEN_VERSION                 "1.4.10"
#define OPEN_VER                               _V(HW_LITEOS_OPEN_VERSION)

#define MAJ_V                            1
#define MIN_V                            4
#define REL_V                            10

#define VERSION_NUM(a,b,c) ((a) << 16|(b) << 8|(c))
#define HW_LITEOS_OPEN_VERSION_NUM VERSION_NUM(MAJ_V,MIN_V,REL_V)

/****************************** Dynamic loading module configuration **************************/
#define OS_AUTOINIT_DYNLOADER                    YES

/****************************** exception information  configuration ******************************/
#ifdef LOSCFG_SHELL_EXCINFO
/**
 * @ingroup los_config
 * the size of space for recording exception information
 */
#define EXCINFO_RECORD_BUF_SIZE    (4*1024)

/**
 * @ingroup los_config
 * the address of space for recording exception information
 */
#define EXCINFO_RECORD_ADDR    (0xa00000)

/**
 *@ingroup los_config
 *@brief  define the type of functions for reading or writing exception information .
 *
 *@par Description:
 *<ul>
 *<li>This defination is used to declare the type of functions for reading or writing exception information</li>
 *</ul>
 *@attention 
 *<ul>
 *<li> 'uwStartAddr' must be left to save the exception address space, the size of 'pBuf' is 'uwSpace'  </li>
 *</ul>
 *
 *@param uwStartAddr    [IN] Address of storage ,its must be left to save the exception address space
 *@param uwSpace          [IN] size of storage.its is also the size of 'pBuf'
 *@param uwRWFlag       [IN] writer-read flag, 0 for writing,1 for reading, other number is to do nothing.
 *@param pBuf                [IN] the buffer of storing data.
 *
 *@retval none.
 *@par Dependency:
 *<ul><li>los_config.h: the header file that contains the type defination.</li></ul>
 *@see
 *@since Huawei LiteOS V100R002C10
 */
typedef VOID (*log_read_write_fn)( UINT32 uwStartAddr, UINT32 uwSpace, UINT32 uwRWFlag, CHAR * pBuf);

/**
 *@ingroup los_config
 *@brief Register recording exception information function.
 *
 *@par Description:
 *<ul>
 *<li>This API is used to Register recording exception information function,and specify location and space and size</li>
 *</ul>
 *@attention
 *<ul>
 *<li> 'uwStartAddr' must be left to save the exception address space, the size of 'pBuf' is 'uwSpace' </li>
 *</ul>
 *
 *@param uwStartAddr    [IN] Address of storage .its must be left to save the exception address space
 *@param uwSpace         [IN] size of storage space.its is also the size of 'pBuf'
 *@param pBuf               [IN] the buffer of storing exception information.
 *@param pfnHook         [IN] the function for reading or writing exception information .
 *
 *@retval none.
 *@par Dependency:
 *<ul><li>los_config.h: the header file that contains the API declaration.</li></ul>
 *@see
 *@since Huawei LiteOS V100R002C10
 */
VOID LOS_ExcInfoRegHook(UINT32 uwStartAddr, UINT32 uwSpace, CHAR * pBuf,  log_read_write_fn pfnHook);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


#endif /* _LOS_CONFIG_H */
