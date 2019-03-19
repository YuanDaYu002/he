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

#include "los_event.h"
#include "hisoc/uart.h"

EVENT_CB_S g_uartEvent;
#ifdef LOSCFG_PLATFORM_UART_WITHOUT_VFS
#ifdef LOSCFG_SHELL
#define UART_BUF    128

static UINT8    g_uart_buf[UART_BUF];
extern void shellCmdLineParse(CHAR c);

static channel_data_t smdk_ser_channels[2] = {
    {(UINT32)UART0_REG_BASE, 1000, NUM_HAL_INTERRUPT_UART0},
    {(UINT32)UART1_REG_BASE, 1000, NUM_HAL_INTERRUPT_UART1}
};
#endif
#endif

UINT8 uart_getc(void)
{
    UINT8 ch = 0;
    UINT32 status;
    UINT32 base = UART_REG_BASE;

    /* Wait until there is data in the FIFO */
    while (READ_UINT32(status, base + UART_FR) & 0x10) {
        LOS_Msleep(100);
    }

    READ_UINT8(ch, base + UART_DR);
    return ch;  /*lint !e438*/
}   /*lint !e550*/

char uart_putc( char c)
{
    UINT32 base = UART_REG_BASE;
    UINT32 status;

    // Wait for Tx FIFO not full
    do {
        READ_UINT32(status, base + UART_FR);
    } while (status & 0x20) ;
    WRITE_UINT8(c, base + UART_DR);
    return c;
}

unsigned int g_uart_fputc_en __attribute__ ((section(".data")))= 1;
char uart_fputc(char c,void *f)
{
    if(g_uart_fputc_en == 1) {
        if (c == '\n')
        {
            uart_putc('\r'); /*lint !e534*/
        }
        return (uart_putc(c));
    } else {
        return 0;
    }
}

#ifdef LOSCFG_PLATFORM_UART_WITHOUT_VFS
#ifdef LOSCFG_SHELL
static void uart_notice_adapt(void)
{
     LOS_EventWrite(&g_uartEvent, 0x112);
}

void uart_get_raw(void)
{
    UINT8 ch;
    static int cnt_ii = 0;
    if(cnt_ii == 0)
    {
        memset(g_uart_buf, 0, UART_BUF);
    }
    ch = uart_getc();
    g_uart_buf[cnt_ii]=ch;
    cnt_ii++;
    switch(cnt_ii)
    {
        case 1: //only one char
            if(ch != 27)  // un special
            {
                uart_notice_adapt();
                cnt_ii =0;
            }
            break;
        case 2:
            if(ch != 91)
            {
                uart_notice_adapt();
                cnt_ii =0;
            }
            break;
        case 3:
            switch(ch)
            {
                default:
                case 'A':
                case 'B':
                case 'C':
                case 'D':
                        uart_notice_adapt();
                        cnt_ii =0;
                    break;
                case 51:
                case 49:
                case 52:
                    break;
            }
            break;
        case 4:
                uart_notice_adapt();
                cnt_ii =0;
            break;
        default:
                uart_notice_adapt();
                cnt_ii =0;
            break;
    }
}

static void uart_irqhandle(void)
{
    shellCmdLineParse(0);
    WRITE_UINT32((1 << 4) | (1 << 6), UART_REG_BASE + UART_CLR);
}

int uart_hwiCreate(void)
{
    UINT32 uwRet = 0;
    if (uwRet != LOS_HwiCreate(NUM_HAL_INTERRUPT_UART, 0xa0, 0, uart_irqhandle, 0))
    {
        return uwRet;
    }
    uart_interrupt_unmask();
    return 0;
}
#endif /* LOSCFG_SHELL */
#endif /* LOSCFG_PLATFORM_UART_WITHOUT_VFS */
void uart_init(void)
{
    UINT32 tmp;
    UINT32 divider;
    UINT32 remainder;
    UINT32 fraction;

    UINT32 base = UART_REG_BASE;

    /* First, disable everything */
    WRITE_UINT32(0x0, base + UART_CR);

    /* set baud rate */
    tmp = 16 * CONSOLE_UART_BAUDRATE ;
    divider = CONFIG_UART_CLK_INPUT /tmp;
    remainder = CONFIG_UART_CLK_INPUT %tmp;
    tmp = (8 * remainder)/CONSOLE_UART_BAUDRATE ;
    fraction = (tmp >> 1) + (tmp & 1);

    WRITE_UINT32(divider, base + UART_IBRD);
    WRITE_UINT32(fraction, base + UART_FBRD);

    /* Set the UART to be 8 bits, 1 stop bit, no parity, fifo enabled. */
    WRITE_UINT32((3 << 5) | (1 << 4), base + UART_LCR_H);

    /*set the fifo threshold of recv interrupt >= 1/8 full */
    WRITE_UINT32((2 << 3) | (4 << 0), base + UART_IFLS);

    /* set nonblock of recv interrupt and recv timeout interrupt */
    WRITE_UINT32((1 << 4) | (1 << 6), base + UART_IMSC);
    /* enable the UART */
    WRITE_UINT32((1 << 0) | (1 << 8) | (1 << 9), base + UART_CR);

    LOS_EventInit(&g_uartEvent);    /*lint !e534*/
}

void uart_interrupt_unmask(void)
{
    hal_interrupt_unmask(NUM_HAL_INTERRUPT_UART);
}
