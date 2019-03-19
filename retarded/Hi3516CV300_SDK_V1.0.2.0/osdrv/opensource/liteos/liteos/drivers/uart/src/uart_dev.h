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

#ifndef __UART_DEV_H__
#define __UART_DEV_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifndef FALSE
#define FALSE    (0)
#endif

#ifndef TRUE
#define TRUE    (1)
#endif



#define BUF_SIZE    0x4000    /* receive buf default size: 16K */
//#define TX_BUF_SIZE    0x4000    /* send buf default size: 16K */
#define CONFIG_MAX_BAUDRATE    921600 /* max baud rate */


/* trace */
#define  TRACE_INIT    (1<<0)
#define  TRACE_DATA    (1<<2)
#define  TRACE_MSG     (1<<3)
#define  TRACE_ERR     (1<<4)

#define TRACE_MASK     (0xff)
/* messages print definitions, for debug ,err e.g. */
#ifdef LOSCFG_DEBUG_VERSION
    #define uart_trace(mask, msg...) do { \
        if ((mask) & TRACE_MASK) { \
            dprintf("<uart>,%s:%d: ", __func__, __LINE__); \
            dprintf(msg); \
            dprintf("\n"); \
        } \
    } while (0)
#else
    #define uart_trace(mask, msg...)
#endif

#define uart_error(msg...) do { \
    dprintf("<uart,err>:%s,%d: ", __func__, __LINE__); \
    dprintf(msg); \
    dprintf("\n"); \
}while(0)

struct uart_driver_data;

struct uart_ops {
    //int (*init)(struct uart_driver_data *udd);
    //int (*de_init)(struct uart_driver_data *udd);
    int (*startup)(struct uart_driver_data *udd);
    int (*shutdown)(struct uart_driver_data *udd);

    int (*dma_startup)(struct uart_driver_data *udd, int dir);
    int (*dma_shutdown)(struct uart_driver_data *udd, int dir);
#define UART_DMA_DIR_RX    0
#define UART_DMA_DIR_TX    1

    int (*start_tx)(struct uart_driver_data *udd,
            const char *buf, size_t count);

    int (*config)(struct uart_driver_data *udd);
    /* private operation */
    int (*priv_operator)(struct uart_driver_data *udd, void *data);
};

struct uart_ioc_transfer {
    unsigned int rp;
    unsigned int wp;
    unsigned int flags;
#define BUF_CIRCLED    (1 << 0)
#define BUF_OVERFLOWED    (1 << 1)
#define BUF_EMPTIED    (1 << 2)

    char data[BUF_SIZE];
};

struct __uart_attr {
    unsigned int data_bits : 4;    /* bit0~3: data bits */
#define UART_ATTR_DATABIT_8    0
#define UART_ATTR_DATABIT_7    1
#define UART_ATTR_DATABIT_6    2
#define UART_ATTR_DATABIT_5    3

    unsigned int parity : 4;      /* bit4~7: parity */
#define UART_ATTR_PARITY_NONE    0
#define UART_ATTR_PARITY_ODD    1
#define UART_ATTR_PARITY_EVEN    2
#define UART_ATTR_PARITY_MARK    3
#define UART_ATTR_PARITY_SPACE    4

    unsigned int stop_bits : 4;   /* bit8~11: stop bits */
#define UART_ATTR_STOPBIT_1    0
#define UART_ATTR_STOPBIT_1P5    1
#define UART_ATTR_STOPBIT_2    2

    unsigned int rts : 1;    /* bit 12: rts */
#define UART_ATTR_RTS_DIS    0
#define UART_ATTR_RTS_EN    1

    unsigned int cts : 1;    /* bit 13: cts */
#define UART_ATTR_CTS_DIS    0
#define UART_ATTR_CTS_EN    1

    unsigned int fifo_rx_en : 1;    /* bit 14: rx fifo enable */
#define UART_ATTR_RX_FIFO_DIS    0
#define UART_ATTR_RX_FIFO_EN    1

    unsigned int fifo_tx_en : 1;    /* bit 15: tx fifo enable */
#define UART_ATTR_TX_FIFO_DIS    0
#define UART_ATTR_TX_FIFO_EN    1

    unsigned int reserved : 16;    /* bit16~31: reserved */
};

typedef int (*recv_notify)(struct uart_driver_data *udd,
        const char *buf, size_t count);

struct uart_driver_data {
    unsigned int num;
    unsigned int baudrate;
    struct __uart_attr attr;
    struct uart_ioc_transfer *rx_transfer;
    wait_queue_head_t wait;
	int count;
    int state;
#define UART_STATE_NOT_OPENED   (0)
#define UART_STATE_OPENING       (1)
#define UART_STATE_USEABLE       (2)
#define UART_STATE_SUSPENED     (3)

    unsigned int flags;
#define UART_FLG_DMA_RX   (1 << 0)
#define UART_FLG_DMA_TX   (1 << 1)
#define UART_FLG_RD_BLOCK   (1 << 2)

    recv_notify recv;
    struct uart_ops *ops;
    void *private;
};

/*
 * uart functions which are in uart host driver
 * uart host driver must define them
 * */
/* get uart host numbers */
int get_uart_num(void);
/* get uart driver data(udd) which is defined in uart host drivers,
 * through uart number */
struct uart_driver_data *get_uart_drv_data(int num);


/*
 * uart core functions
 * */
/* read some data from rx_data buf in uart_ioc_transfer */
int uart_dev_read(struct uart_driver_data *udd, char *buf, unsigned int count);
/* check the buf is empty */
int uart_rx_buf_empty(struct uart_driver_data *udd);

int uart_recv_notify(struct uart_driver_data *udd, const char *buf, size_t count);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __UART_DEV_H__ */
