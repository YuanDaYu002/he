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

#include "los_tick.ph"
#include "los_hwi.h"
#include "hisoc/timer.h"
#include "hisoc/cpu.h"
#include "hisoc/sys_ctrl.h"
#include "los_memory.h"
#include "hisoc/random.h"
#include "los_hw_tick_minor.h"
#ifdef LOSCFG_LIB_LIBC
#include "string.h"
#endif /* LOSCFG_LIB_LIBC */
#ifdef LOSCFG_COMPAT_LINUX
#include "asm/delay.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


void arm926_dma_clean_range(int start, int end);
void arm926_dma_inv_range(int start, int end);

void dma_cache_clean(int start, int end)
{
    arm926_dma_clean_range(start,end);
}
void dma_cache_inv(int start, int end)
{
    arm926_dma_inv_range(start,end);
}

unsigned long g_usb_mem_addr_start __attribute__ ((section(".data"))) = SYS_MEM_BASE + SYS_MEM_SIZE_DEFAULT;
unsigned long g_usb_mem_size __attribute__ ((section(".data")))= USB_MEM_SIZE;
/*lint -e40*/
__attribute__((weak)) void board_config(void)
{
    g_sys_mem_addr_end = SYS_MEM_BASE + SYS_MEM_SIZE_DEFAULT;
    g_usb_mem_addr_start = g_sys_mem_addr_end;
    g_usb_mem_size = USB_MEM_SIZE;
}

static UINT32 timestamp;
static UINT32 lastdec;

#define READ_TIMER    (*(volatile unsigned long *) \
                (TIMER_TICK_REG_BASE + TIMER_VALUE))

VOID reset_timer_masked(VOID)
{
    lastdec = READ_TIMER;
    timestamp = 0;
}

UINT32 get_timer_masked(VOID)
{
    UINT32 now = READ_TIMER;

    if (lastdec >= now) {
        /* not roll back */
        timestamp += lastdec - now;
    } else {
        /* rollback */
        timestamp += lastdec + (g_uwSysClock/LOSCFG_BASE_CORE_TICK_PER_SECOND) - now;
    }

    lastdec = now;
    return timestamp;
}

VOID timer1_init(VOID)
{
    UINT32 temp;

    /* enable timer1 here,*/
    READ_UINT32(temp, SYS_CTRL_REG_BASE + REG_SC_CTRL);
    temp |= TIMER1_ENABLE;
    WRITE_UINT32(temp, SYS_CTRL_REG_BASE + REG_SC_CTRL);

    /* disable first */
    WRITE_UINT32(0x0, TIMER1_REG_BASE + TIMER_CONTROL);

    /* set init value as 0xFFFFFFFF */
    WRITE_UINT32(0xFFFFFFFF, TIMER1_REG_BASE + TIMER_LOAD);

    /*
     * Timing mode: 32bits [bit 1 set as 1]
     * ticking with 1/16 clock frequency [bit 2 set as 1, bit 3 set as 0]
     * timer enabled [bit 7 set as 1]
     * timer is oneshot [bit 0 set as 1]
     */
    temp = (1 << 2) | (1 << 1) | (1 << 0);
    WRITE_UINT32(temp, TIMER1_REG_BASE + TIMER_CONTROL);
}

VOID timer1_enable(VOID)
{
    GET_UINT32(TIMER1_REG_BASE + TIMER_CONTROL) = GET_UINT32(TIMER1_REG_BASE + TIMER_CONTROL) | (1 << 7);
}

VOID timer1_disable(VOID)
{
    GET_UINT32(TIMER1_REG_BASE + TIMER_CONTROL) = GET_UINT32(TIMER1_REG_BASE + TIMER_CONTROL) | (0 << 7);
}

VOID hrtimer_clock_irqclear(VOID)
{
    WRITE_UINT32(1, HRTIMER_TIMER_REG_BASE + TIMER_INT_CLR);
}

VOID hrtimer_clock_initialize(VOID)
{
    UINT32 temp;

    /*
    *enable timer here,
    */
    READ_UINT32(temp, SYS_CTRL_REG_BASE + REG_SC_CTRL);
    temp |= HRTIMER_TIMER_ENABLE;
    WRITE_UINT32(temp, SYS_CTRL_REG_BASE + REG_SC_CTRL);

    /* disable timer */
    WRITE_UINT32(0x0, HRTIMER_TIMER_REG_BASE + TIMER_CONTROL);

    /*
     * Timing mode:oneshot [bit 0 set as 1]
     * timersize:32bits [bit 1 set as 1]
     * ticking with 1/1 clock frequency [bit 3 set as 0, bit 2 set as 0]
     * interrupt enabled [bit 5 set as 1]
     * timing circulary [bit 6 set as 1]
     */
     temp = (1 << 6) | (1 << 5) | (1 << 1) | (1 << 0);
     WRITE_UINT32(temp, HRTIMER_TIMER_REG_BASE + TIMER_CONTROL);
}

VOID hrtimer_clock_start(UINT32 period)
{
    UINT32 temp;

    /* set init value as period */
    WRITE_UINT32(period, HRTIMER_TIMER_REG_BASE + TIMER_LOAD);

    /*timer enabled [bit 7 set as 1]*/
    READ_UINT32(temp, HRTIMER_TIMER_REG_BASE + TIMER_CONTROL);
    temp |= 1 << 7;
    WRITE_UINT32(temp, HRTIMER_TIMER_REG_BASE + TIMER_CONTROL);
}

VOID hrtimer_clock_stop(VOID)
{
    UINT32 temp;

     /*timer disabled [bit 7 set as 0]*/
    READ_UINT32(temp, HRTIMER_TIMER_REG_BASE + TIMER_CONTROL);
    temp &= ~(1 << 7);
    WRITE_UINT32(temp, HRTIMER_TIMER_REG_BASE + TIMER_CONTROL);
}

UINT32 get_hrtimer_clock_value(VOID)
{
    UINT32 temp;

    /*Read the current value of the timer3*/
    READ_UINT32(temp, HRTIMER_TIMER_REG_BASE + TIMER_VALUE);
    return temp;
}

UINT32 time_clk_read(VOID)
{
    UINT32 value;

    READ_UINT32(value, TIMER_TIME_REG_BASE + TIMER_VALUE);
    value = 0UL - value;

    return value;
}
unsigned int arch_timer_rollback(VOID)
{
    UINT32 flag;

    READ_UINT32(flag, TIMER_TICK_REG_BASE + TIMER_RIS);
    return flag;
}

//this func is start timer2 for start time
VOID hal_clock_initialize_start(VOID)
{
    UINT32 temp;
    /*
     * enable timer2 here.
     */
    READ_UINT32(temp, SYS_CTRL_REG_BASE + REG_SC_CTRL);
    temp |= TIMER2_ENABLE;
    WRITE_UINT32(temp, SYS_CTRL_REG_BASE + REG_SC_CTRL);

    /*
     * Timing mode: 32bits [bit 1 set as 1]
     * ticking with 1/256 clock frequency [bit 3 set as 1, bit 2 set as 0]
     * timing circulary [bit 6 set as 1]
     * timer enabled [bit 7 set as 1]
     */

    /* init the timestamp and lastdec value */
    reset_timer_masked();

    /* disable timer2 */
    WRITE_UINT32(0x0, TIMER2_REG_BASE + TIMER_CONTROL);
    /* set init value as period */
    WRITE_UINT32(0xffffffff, TIMER2_REG_BASE + TIMER_LOAD);

    /*
     * Timing mode: 32bits [bit 1 set as 1]
     * ticking with 1/256 clock frequency [bit 3 set as 1, bit 2 set as 0]
     * timing circulary [bit 6 set as 1]
     * timer enabled [bit 7 set as 1]
     */
    temp = (1 << 7) | (1 << 6) | (1 << 3) | (1 << 1);
    WRITE_UINT32(temp, TIMER2_REG_BASE + TIMER_CONTROL);
}

UINT32 GetTimer2Value(VOID)
{
    UINT32 temp;

    READ_UINT32(temp, TIMER2_REG_BASE + TIMER_VALUE);
    return temp;
}

/*
 * get the system ms clock since the system start
 */
UINT32 hi_getmsclock(VOID)
{
    UINT32 t32 = 0xffffffff - GetTimer2Value();
    UINT64 t64 = (UINT64)t32 * 256;

    return (UINT32)(t64 / (get_bus_clk()/1000));
}

VOID los_bss_init(unsigned int bss_start,unsigned int bss_end)
{
    memset((unsigned int *)bss_start, 0, bss_end - bss_start);
}

VOID hal_clock_initialize(UINT32 period)
{
    UINT32 temp;
    UINT32 uwRet = 0;
    /*
     * enable time0, timer1 here,
     * but only time0 is used for system clock.
     */
    READ_UINT32(temp, SYS_CTRL_REG_BASE + REG_SC_CTRL);
    temp |= TIMER_TICK_ENABLE | TIMER_TIME_ENABLE;
    WRITE_UINT32(temp, SYS_CTRL_REG_BASE + REG_SC_CTRL);

    /* disable first */
    WRITE_UINT32(0x0, TIMER_TICK_REG_BASE + TIMER_CONTROL);

    /* set init value as period */
    WRITE_UINT32(period, TIMER_TICK_REG_BASE + TIMER_LOAD);

    /*
     * Timing mode: 32bits [bit 1 set as 1]
     * ticking with 1/1 clock frequency [bit 3 set as 0, bit 2 set as 0]
     * timing circulary [bit 6 set as 1]
     */
    temp = (1 << 6) | (1 << 5) | (1 << 1);
    WRITE_UINT32(temp, TIMER_TICK_REG_BASE + TIMER_CONTROL);

    /* init the timestamp and lastdec value */
    reset_timer_masked();

    /* disable timer1 */
    WRITE_UINT32(0x0, TIMER_TIME_REG_BASE + TIMER_CONTROL);
    /* set init value as period */
    WRITE_UINT32(0xffffffff, TIMER_TIME_REG_BASE + TIMER_LOAD);

    /*
     * Timing mode: 32bits [bit 1 set as 1]
     * ticking with 1/1 clock frequency [bit 3 set as 0, bit 2 set as 0]
     * timing circulary [bit 6 set as 1]
     * timer enabled [bit 7 set as 1]
     */
    temp = (1 << 7) | (1 << 6) | (1 << 1);
    WRITE_UINT32(temp, TIMER_TIME_REG_BASE + TIMER_CONTROL);

    if (uwRet != LOS_HwiCreate(NUM_HAL_INTERRUPT_TIMER, 0xa0, 0, osTickHandler, 0)) {
    }
}
VOID hal_clock_irqclear(VOID)
{
    WRITE_UINT32(1, TIMER_TICK_REG_BASE + TIMER_INT_CLR);
}

VOID hal_clock_enable(VOID)
{
    GET_UINT32(TIMER_TICK_REG_BASE + TIMER_CONTROL) = GET_UINT32(TIMER_TICK_REG_BASE + TIMER_CONTROL) | (1 << 7);
}

// This routine is called during a clock interrupt.
VOID hal_clock_reset(UINT32 vector, UINT32 period)
{
    UINT32 temp;

    /* clear timer interrupt */
    WRITE_UINT32(0xffffffff, TIMER_TICK_REG_BASE + TIMER_INT_CLR);

    /* disable first */
    WRITE_UINT32(0x0, TIMER_TICK_REG_BASE + TIMER_CONTROL);
    /* set init value as period */
    WRITE_UINT32(period, TIMER_TICK_REG_BASE + TIMER_LOAD);

    /*
     * Timing mode: 32bits mode [bit 1 set as 1]
     * ticking with 1/256 clock frequency [bit 3 set as 1, bit 2 set as 0]
     * do not mask timer irq[bit 5 set as 1]
     * timing circulary [bit 6 set as 1]
     * timer enabled [bit 7 set as 1]
     */
    temp = (1 << 7) | (1 << 6) | (1 << 5) | (1 << 2) | (1 << 1);
    WRITE_UINT32(temp, TIMER_TICK_REG_BASE + TIMER_CONTROL);
}

// Read the current value of the clock, returning the number of hardware
// "ticks" that have occurred (i.e. how far away the current value is from
// the start)
VOID hal_clock_read(UINT32 *pvalue)
{
    INT32 value;

    READ_UINT32(value, TIMER_TICK_REG_BASE + TIMER_VALUE);
    *pvalue = NUM_HAL_RTC_PERIOD - value;
}

// Delay for some number of micro-seconds

UINT32 get_timer(UINT32 base)
{
    return get_timer_masked() - base;
}

VOID hal_delay_us(UINT32 usecs)
{
    UINT32 tmo = 0;
    UINT32 tmp = 0;
    UINT32 uwRet;

    uwRet = LOS_IntLock();

    if (usecs >= 1000) {
        /* start to normalize for usec to ticks per sec */
        tmo = usecs / 1000;

        /* tmo *= 50000000 / 1000; */
        tmo *= (g_uwSysClock / 1000);//largest msecond 1374389

        /* usecs < 1000 */
        usecs -= (usecs / 1000 * 1000);
    }

    /* usecs < 1000 */
    if(usecs) {
        /*
         * translate us into sys_clock
         * prevent u32 overflow
         * */
        tmo += (usecs * (g_uwSysClock / 1000)) / 1000;
    }

    /* reset "advancing" timestamp to 0, set lastdec value */
    reset_timer_masked();

    tmp = get_timer(0);     /* get current timestamp */

    /* set advancing stamp wake up time */
    tmo += tmp;

    while (get_timer_masked() < tmo) ;
    LOS_IntRestore(uwRet);
}

VOID LOS_Udelay(UINT32 usecs)
{
    hal_delay_us(usecs);
}

VOID LOS_Mdelay(UINT32 msecs)
{
    LOS_Udelay(msecs * 1000);
}

// -------------------------------------------------------------------------

// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.

extern UINT32 g_vuwHwiFormCnt[OS_HWI_MAX_NUM];

typedef VOID (*HWI_PROC_FUNC0)(void);
typedef VOID (*HWI_PROC_FUNC2)(int,void *);
void hal_interrupt_acknowledge(unsigned int vector);

void hal_IRQ_handler(void)
{
    unsigned int i = 0;
    UINT32 irqstatus = 0x0;

    READ_UINT32(irqstatus, IRQ_REG_BASE + IRQ_INT_IRQSTATUS);

    for (i = 0; i < OS_HWI_MAX_NUM && irqstatus; ++i) {
        if (irqstatus & (1 << i)) {
            HWI_HANDLE_FORM_S *pstHwiForm = &m_astHwiForm[i];
            while (NULL != pstHwiForm->pstNext) {
                pstHwiForm = pstHwiForm->pstNext;
                if (!pstHwiForm->uwParam) {
                    HWI_PROC_FUNC0 func0 = (HWI_PROC_FUNC0)(pstHwiForm->pfnHook);
                    func0();
                } else {
                    HWI_PROC_FUNC2 func2 = (HWI_PROC_FUNC2)(pstHwiForm->pfnHook);
                    unsigned int *p = (unsigned int *)(pstHwiForm->uwParam);
                    func2((int)(*p), (void *)(*(p + 1)));
                }
                ++g_vuwHwiFormCnt[i];

            }
            hal_interrupt_acknowledge(i);
            irqstatus &= irqstatus - 1;
        }
    }
}

//----------------------------------------------------------------------------
// Interrupt control
VOID hal_interrupt_mask(unsigned int vector)
{
    UINT32 imr = 1 << vector;

    if (vector > OS_USER_HWI_MAX || vector < OS_USER_HWI_MIN) /*lint !e685 !e568*/
    {
        return;
    }

    WRITE_UINT32(imr, IRQ_REG_BASE + IRQ_INT_INTENCLEAR);
}

VOID hal_interrupt_unmask(unsigned int vector)
{
    UINT32 imr = 1 << vector;
    UINT32 status = 0;

    if (vector > OS_USER_HWI_MAX || vector < OS_USER_HWI_MIN) /*lint !e685 !e568*/
    {
        return;
    }

    READ_UINT32(status, IRQ_REG_BASE + IRQ_INT_INTENABLE);
    WRITE_UINT32(imr | status, IRQ_REG_BASE + IRQ_INT_INTENABLE);
}

VOID hal_interrupt_acknowledge(unsigned int vector)
{
    UINT32 imr = 1 << vector;

    if (vector > OS_USER_HWI_MAX || vector < OS_USER_HWI_MIN) /*lint !e685 !e568*/
    {
        return;
    }

    WRITE_UINT32(imr, IRQ_REG_BASE + IRQ_INT_SOFTINTCLEAR);
}

VOID hal_interrupt_configure(unsigned int vector, int level, int up)
{
}

VOID hal_interrupt_pending(unsigned int vector)
{
    UINT32 imr = 1 << vector;
    UINT32 status = 0;

    if (vector > OS_USER_HWI_MAX || vector < OS_USER_HWI_MIN) /*lint !e685 !e568*/
    {
        return;
    }

    READ_UINT32(status, IRQ_REG_BASE + IRQ_INT_SOFTINT);
    WRITE_UINT32(imr | status, IRQ_REG_BASE + IRQ_INT_SOFTINT);
}

void hal_interrupt_set_level(unsigned int vector, int level)
{
}

void hal_interrupt_init(void)
{
    WRITE_UINT32(0xffffffff, IRQ_REG_BASE + IRQ_INT_INTENCLEAR);    /*mask all irq*/
    WRITE_UINT32(0x00000000, IRQ_REG_BASE + IRQ_INT_INTSELECT);     /*types of all interrupt are IRQ*/
}

void __power_off(void)
{
    unsigned int gpio_group, gpio_offset;

    gpio_group = 13;
    gpio_offset = 2;

    /*config gpio is output*/
    WRITE_UINT32(0x01 << gpio_offset, IO_ADDRESS(GPIO0_REG_BASE + (gpio_group << 16) + 0x400));

    /*output high*/
    WRITE_UINT32(0x01 << gpio_offset,
        IO_ADDRESS(GPIO0_REG_BASE + (gpio_group << 16) + 0x00 + (1 << (gpio_offset +  2))));
}
/*restore*/

/*TODO: use temporary for IDE*/
void __aeabi_read_tp(void) {}

UINT32 random_stack_guard(VOID)
{
    UINT32 result;
    int ret;

    ret = hi_random_hw_init();
    if (ret < 0) PRINT_ERR("hi_random_hw_init Error!\n");

    ret = hi_random_hw_getinteger(&result);
    if (ret < 0)
    {
        PRINT_ERR("hi_random_hw_getinteger Error!\n");
        result = SysTick->uwVALUE;
    }

    ret = hi_random_hw_deinit();
    if (ret < 0) PRINT_ERR("hi_random_hw_deinit Error!\n");

    return result;
}

VOID tick_interrupt_mask(VOID)
{
    hal_interrupt_mask(NUM_HAL_INTERRUPT_TIMER);
}

VOID tick_interrupt_unmask(VOID)
{
    hal_interrupt_unmask(NUM_HAL_INTERRUPT_TIMER);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
