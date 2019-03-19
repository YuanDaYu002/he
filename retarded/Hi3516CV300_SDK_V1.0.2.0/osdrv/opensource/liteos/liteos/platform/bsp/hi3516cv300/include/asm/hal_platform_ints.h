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

#ifndef PLATFORM_HAL_PLATFORM_INTS_H
#define PLATFORM_HAL_PLATFORM_INTS_H

#include"los_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * Maximum number of supported hardware devices that generate hardware interrupts.
 * The maximum number of hardware devices that generate hardware interrupts supported by hi3516cv300 is 32.
 */
#define OS_HWI_MAX_NUM        32

/**
 * Maximum interrupt number.
 */

#define OS_HWI_MAX            ((OS_HWI_MAX_NUM) - 1)

/**
 * Minimum interrupt number.
 */

#define OS_HWI_MIN            0

/**
 * Maximum usable interrupt number.
 */

#define OS_USER_HWI_MAX            OS_HWI_MAX

/**
 * Minimum usable interrupt number.
 */

#define OS_USER_HWI_MIN            OS_HWI_MIN


#define NUM_HAL_INTERRUPT_SOFTINT    0
#define NUM_HAL_INTERRUPT_WDT        1

#define NUM_HAL_INTERRUPT_TIMER0     3
#define NUM_HAL_INTERRUPT_TIMER1     3
#define NUM_HAL_INTERRUPT_TIMER2     4
#define NUM_HAL_INTERRUPT_TIMER3     4
#define NUM_HAL_INTERRUPT_UART0         5
#define NUM_HAL_INTERRUPT_SSP0         6
#define NUM_HAL_INTERRUPT_SSP1         7
//SGZIP and DDRT share interrupt num
#define NUM_HAL_INTERRUPT_SGZIP         8
#define NUM_HAL_INTERRUPT_DDRT          8
#define NUM_HAL_INTERRUPT_AIO         9
#define NUM_HAL_INTERRUPT_USB_DEV    10
#define NUM_HAL_INTERRUPT_FMC         11 //FMC and EMMC share interrupt num
#define NUM_HAL_INTERRUPT_EMMC         11
#define NUM_HAL_INTERRUPT_ETH         12
#define NUM_HAL_INTERRUPT_CIPHER     13
#define NUM_HAL_INTERRUPT_DMAC         14
#define NUM_HAL_INTERRUPT_USB_EHCI     15
#define NUM_HAL_INTERRUPT_USB_OHCI     16
#define NUM_HAL_INTERRUPT_VPSS         17
#define NUM_HAL_INTERRUPT_SDIO0         18
#define NUM_HAL_INTERRUPT_IR         19
#define NUM_HAL_INTERRUPT_SAR_ADC     19
#define NUM_HAL_INTERRUPT_I2C           20
#define NUM_HAL_INTERRUPT_IVE         21
#define NUM_HAL_INTERRUPT_VICAP         22
#define NUM_HAL_INTERRUPT_VDP         23
#define NUM_HAL_INTERRUPT_VEDU         24
#define NUM_HAL_INTERRUPT_UART2         25
#define NUM_HAL_INTERRUPT_JPGE         26
#define NUM_HAL_INTERRUPT_SDIO1      27
#define NUM_HAL_INTERRUPT_SDIO3      27
#define NUM_HAL_INTERRUPT_MIPI         28
#define NUM_HAL_INTERRUPT_VGS         29
#define NUM_HAL_INTERRUPT_UART1         30
#define NUM_HAL_INTERRUPT_GPIO         31
#define NUM_HAL_INTERRUPT_TIMER   NUM_HAL_INTERRUPT_TIMER0
#define NUM_HAL_INTERRUPT_HRTIMER   NUM_HAL_INTERRUPT_TIMER3

#define NUM_HAL_INTERRUPT_NONE    -1

#define NUM_HAL_ISR_MIN            32
#define NUM_HAL_ISR_MAX            1020
#define NUM_HAL_ISR_COUNT          (NUM_HAL_ISR_MAX-NUM_HAL_ISR_MIN+1)

// The vector used by the Real time clock
#define NUM_HAL_INTERRUPT_RTC      NUM_HAL_INTERRUPT_TIMER0

#define NUM_HAL_RTC_PERIOD 9999
#define NUM_HAL_ISR_MAX 1020
#define NUM_HAL_ISR_MIN 32


#define IO_ADDRESS(x)       (x)

#define HAL_READ_UINT8(addr, data)  READ_UINT8(data, addr)

#define HAL_WRITE_UINT8(addr, data) WRITE_UINT8(data, addr)

#define HAL_READ_UINT32(addr, data) READ_UINT32(data, addr)

#define HAL_WRITE_UINT32(addr, data) WRITE_UINT32(data, addr)

VOID hal_interrupt_mask(unsigned int vector);
VOID hal_interrupt_unmask(unsigned int vector);
VOID hal_interrupt_init(VOID);

extern VOID hrtimer_clock_irqclear(VOID);
extern VOID hrtimer_clock_start(UINT32 period);
extern VOID hrtimer_clock_stop(VOID);
extern UINT32 get_hrtimer_clock_value(VOID);
extern VOID hrtimer_clock_initialize(VOID);

VOID tick_interrupt_mask(VOID);
VOID tick_interrupt_unmask(VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif // PLATFORM_HAL_PLATFORM_INTS_H
