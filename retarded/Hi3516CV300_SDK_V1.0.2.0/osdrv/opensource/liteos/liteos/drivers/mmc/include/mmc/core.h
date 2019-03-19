#ifndef MMC_CORE_H
#define MMC_CORE_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

struct mmc_request;

#define MMC_CMD_RETRIES        3

struct mmc_cmd {
    uint32_t            cmd_code;
    uint32_t            arg;
    uint32_t            resp[4];
    unsigned int        resp_type;        /* expected response type */

#define RESP_PRESENT    (1 << 0)
#define RESP_136    (1 << 1)        /* 136 bit response */
#define RESP_CRC    (1 << 2)        /* expect valid crc */
#define RESP_BUSY    (1 << 3)       /* card may send busy */
#define RESP_CMDCODE    (1 << 4)     /* response contains opcode */

#define MMC_CMD_MASK    (3 << 5)        /* non-SPI command type */
#define MMC_CMD_AC    (0 << 5)
#define MMC_CMD_ADTC    (1 << 5)
#define MMC_CMD_BC    (2 << 5)
#define MMC_CMD_BCR    (3 << 5)

#define MMC_RESP_SPI_S1    (1 << 7)        /* one status byte */
#define MMC_RESP_SPI_S2    (1 << 8)        /* second byte */
#define MMC_RESP_SPI_B4    (1 << 9)        /* four data bytes */
#define MMC_RESP_SPI_BUSY (1 << 10)        /* card may send busy */
#define MMC_CMD_NON_BLOCKING    (1 << 11)
#define MMC_CMD_TYPE_RW     (1 << 12)

#define MMC_RESP_NONE    (0)
#define MMC_RESP_R1    (RESP_PRESENT|RESP_CMDCODE|RESP_CRC)
#define MMC_RESP_R1B   (RESP_PRESENT|RESP_CMDCODE|RESP_BUSY|RESP_CRC)
#define MMC_RESP_R2    (RESP_PRESENT|RESP_136|RESP_CRC)
#define MMC_RESP_R3    (RESP_PRESENT)
#define MMC_RESP_R4    (RESP_PRESENT)
#define MMC_RESP_R5    (RESP_PRESENT|RESP_CMDCODE|RESP_CRC)
#define MMC_RESP_R6    (RESP_PRESENT|RESP_CMDCODE|RESP_CRC)
#define MMC_RESP_R7    (RESP_PRESENT|RESP_CMDCODE|RESP_CRC)

#define mmc_resp_type(cmd)    ((cmd)->resp_type  & (RESP_PRESENT|RESP_136|RESP_CRC|RESP_BUSY|RESP_CMDCODE))

#define MMC_RESP_SPI_R1    (MMC_RESP_SPI_S1)
#define MMC_RESP_SPI_R1B    (MMC_RESP_SPI_S1|MMC_RESP_SPI_BUSY)
#define MMC_RESP_SPI_R2    (MMC_RESP_SPI_S1|MMC_RESP_SPI_S2)
#define MMC_RESP_SPI_R3    (MMC_RESP_SPI_S1|MMC_RESP_SPI_B4)
#define MMC_RESP_SPI_R4    (MMC_RESP_SPI_S1|MMC_RESP_SPI_B4)
#define MMC_RESP_SPI_R5    (MMC_RESP_SPI_S1|MMC_RESP_SPI_S2)
#define MMC_RESP_SPI_R7    (MMC_RESP_SPI_S1|MMC_RESP_SPI_B4)

#define mmc_spi_resp_type(cmd)    ((cmd)->flags & \
        (MMC_RSP_SPI_S1|MMC_RSP_SPI_BUSY|MMC_RSP_SPI_S2|MMC_RSP_SPI_B4))
#define mmc_cmd_type(cmd)    ((cmd)->resp_type & MMC_CMD_MASK)

    unsigned int        retries;    /* max number of retries */
    int        err;        /* command error */

//    unsigned int        timeout_ms;    /* in milliseconds */

//    struct mmc_data     *data;        /* data segment associated with cmd */
//    struct mmc_request    *mrq;        /* associated request */
};

struct mmc_data {
    unsigned int        blocksz;        /* data block size */
    unsigned int        blocks;        /* number of blocks */
    unsigned int        bytes_xfered;
    int        err;        /* data error */
    unsigned int        data_flags;
#define MMC_DATA_WRITE     (1 << 8)
#define MMC_DATA_READ    (1 << 9)
#define MMC_DATA_STREAM    (1 << 10)

#define MMC_CMD_STOP     (1<<5)
#define MMC_CMD_STOPENABLE(data)   ((data)->data_flags & MMC_CMD_STOP)
    unsigned char        *data_buffer;

    unsigned int        timeout_clks;    /* data timeout (in clocks) */
    unsigned int        timeout_ns;    /* data timeout (in ms, max 80ms) */
    struct mmc_cmd    cmd_stop;        /* stop command */
//    struct mmc_request    *mrq;        /* associated request */
    unsigned int        sg_len;        /* size of scatter list */
    struct scatterlist    *sg;        /* I/O scatter list */
};

struct mmc_request {
    struct mmc_cmd    *cmd;
    struct mmc_data    *data;

    void            (*done)(struct mmc_request *);/* completion function */
    unsigned int        wr_pos; /*FIXME*/
    mmc_event mrq_event;
#define MMC_REQUEST_DONE (1<<0)
#define MMC_REQUEST_TIMEOUT HZ*8
};

#define MMC_ERASE_ARG       0x00000000
#define MMC_TRIM_ARG        0x00000001
#define MMC_DISCARD_ARG     0x00000003


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* _MMC_CORE_H */
