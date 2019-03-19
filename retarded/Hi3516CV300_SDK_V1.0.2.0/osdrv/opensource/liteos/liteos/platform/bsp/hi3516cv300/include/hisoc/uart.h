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

#ifndef __HISOC_UART_H__
#define __HISOC_UART_H__

#include "asm/platform.h"
#include "los_typedef.h"
#include "los_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


#define UART_DR                 0x0   // data register
#define UART_RSR                0x04
#define UART_FR                 0x18  // flag register
#define UART_CLR                0x44  // interrupt clear register
#define UART_CR                 0x30  // control register
#define UART_IBRD               0x24  // interge baudrate register
#define UART_FBRD               0x28  // decimal baudrate register
#define UART_LCR_H              0x2C
#define UART_IFLS               0x34  // fifo register
#define UART_IMSC               0x38  // interrupt mask register
#define UART_RIS                0x3C  // base interrupt state register
#define UART_MIS                0x40  // mask interrupt state register
#define UART_DMACR              0x48  // DMA control register

#define CONFIG_UART_CLK_INPUT    ((GET_UINT32(CRG_REG_BASE + 0xE4) & (1<<19))? 6000000:24000000)

#define UART0       0
#define UART1       1
#define UART2       2
#define UART_NUM    3

#define UART0_ENABLE  1
#define UART1_ENABLE  1
#define UART2_ENABLE  1

#define UART0_DMA_RX_PERI       6
#define UART1_DMA_RX_PERI       8
#define UART2_DMA_RX_PERI       10

#define hi_uart_mux_cfg(uart_num) ({\
        if(UART0 == uart_num){\
        WRITE_UINT16(0x01, IO_MUX_REG_BASE + 0x010);\
        WRITE_UINT16(0x01, IO_MUX_REG_BASE + 0x014);\
        }\
        else if(UART1 == uart_num){\
        WRITE_UINT16(0x01, IO_MUX_REG_BASE + 0x088);\
        WRITE_UINT16(0x01, IO_MUX_REG_BASE + 0x090);\
        }\
        else if(UART2 == uart_num){\
        WRITE_UINT16(0x04, IO_MUX_REG_BASE + 0x05C);\
        WRITE_UINT16(0x04, IO_MUX_REG_BASE + 0x060);\
        }\
        })
#define  uart_clk_cfg(uart_num, flag)  ({\
        unsigned int tmp = 0;\
        tmp = GET_UINT32(CRG_REG_BASE + 0x0E4);\
        if(flag)\
            tmp |= (1<<(uart_num+15));\
        else\
            tmp &= ~(1<<(uart_num+15));\
        WRITE_UINT32(tmp, CRG_REG_BASE + 0x0E4);\
        })
#define  get_uart_dma_peri(uart_num) ({\
        unsigned int peri_num;\
        if(UART0 == uart_num)\
        peri_num = UART0_DMA_RX_PERI;\
        else if(UART1 == uart_num)\
        peri_num = UART1_DMA_RX_PERI;\
        else if(UART2 == uart_num)\
        peri_num = UART2_DMA_RX_PERI;\
        peri_num;\
        })


#define DECLARE_UART_PORT(NUM)    struct arm_pl011_port uart_port[NUM] = \
{\
	/* UART0 */\
	{\
		.enable           = UART0_ENABLE,\
		.phys_base        = UART0_REG_BASE,\
		.irq_num          = NUM_HAL_INTERRUPT_UART0,\
		.default_baudrate = DEFAULT_UART0_BAUDRATE,\
		.rx_udt           = NULL,\
		.udd              = NULL,\
	},\
	/* UART1 */\
	{\
		.enable           = UART1_ENABLE,\
		.phys_base        = UART1_REG_BASE,\
		.irq_num          = NUM_HAL_INTERRUPT_UART1,\
		.default_baudrate = DEFAULT_UART1_BAUDRATE,\
		.rx_udt           = NULL,\
		.udd              = NULL,\
	},\
	/* UART2 */\
	{\
		.enable           = UART2_ENABLE,\
		.phys_base        = UART2_REG_BASE,\
		.irq_num          = NUM_HAL_INTERRUPT_UART2,\
		.default_baudrate = DEFAULT_UART2_BAUDRATE,\
		.rx_udt           = NULL,\
		.udd              = NULL,\
	},\
}

#define CONSOLE_UART                        UART0
#define CONSOLE_UART_BAUDRATE               115200

#if (CONSOLE_UART == UART0)
    #define TTY_DEVICE                "/dev/uartdev-0"
    #define UART_REG_BASE             UART0_REG_BASE
    #define NUM_HAL_INTERRUPT_UART    NUM_HAL_INTERRUPT_UART0
#elif (CONSOLE_UART == UART1)
    #define TTY_DEVICE                "/dev/uartdev-1"
    #define UART_REG_BASE             UART1_REG_BASE
    #define NUM_HAL_INTERRUPT_UART    NUM_HAL_INTERRUPT_UART1
#elif (CONSOLE_UART == UART2)
    #define TTY_DEVICE                "/dev/uartdev-2"
    #define UART_REG_BASE             UART2_REG_BASE
    #define NUM_HAL_INTERRUPT_UART    NUM_HAL_INTERRUPT_UART2
#endif

typedef struct {
    UINT32 base;
    INT32 msec_timeout;
    int isr_vector;
} channel_data_t;

extern void uart_init(void);
extern void uart_interrupt_unmask(void);
extern int uart_hwiCreate(void);
extern UINT8 uart_getc(void);
extern char uart_putc(char c);
extern char uart_fputc(char c, void *f);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
