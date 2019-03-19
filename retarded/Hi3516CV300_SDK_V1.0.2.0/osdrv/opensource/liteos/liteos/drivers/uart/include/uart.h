#ifndef __UART_USER_H__
#define __UART_USER_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* baudrate config */
#define UART_CFG_BAUDRATE    0x101

/* DMA CONFIG: receive */
#define UART_DMA_RX_EN    1
#define UART_DMA_RX_DIS    0

#define UART_CFG_DMA_RX    0x102

/* DMA CONFIG: send */
#define UART_DMA_TX_EN    1
#define UART_DMA_TX_DIS    0

#define UART_CFG_DMA_TX    0x103

/* Read Block: */
#define UART_RD_BLOCK    1
#define UART_RD_NONBLOCK    0

#define UART_CFG_RD_BLOCK    0x104

/* ATTRIBUTE CONFIG: data_bits, stop_bits, etc. */
struct uart_attr {
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
/*
 * uart attribute config cmd,
 * parameter should be 'struct uart_init *'
 * */
#define UART_CFG_ATTR    0x105


/* uart private cmmand for uart host */
#define UART_CFG_PRIVATE    0x110

extern int uart_dev_init(void);
extern int uartdev_de_init(void);
extern int uart_suspend(void *data);
extern int uart_resume(void *data);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __UART_USER_H__ */
