/*begin sd_ops.c*/

#include "mmc_core.h"

static int mmc_wait_for_app_cmd(struct mmc_card *card,
        struct mmc_cmd *cmd,
        int retries)
{
    struct mmc_request mrq = {0};
    int i, err = -EIO;

    for (i = 0; i <= retries; i++) {
        err = mmc_app_cmd(card);
        if (err)
            continue;
        memset(&mrq, 0, sizeof(struct mmc_request));
        memset(cmd->resp, 0, sizeof(cmd->resp));
        cmd->retries = 0;
        mrq.cmd = cmd;
        mmc_wait_for_req(card->host, &mrq);
        err = cmd->err;
        if (!cmd->err)
            break;
    }
    return err;
}

int sd_app_set_bus_width(struct mmc_card *card, enum mmc_bus_width width)
{
    struct mmc_cmd cmd = {0};

    cmd.cmd_code = SD_ACMD_SET_BUS_WIDTH;
    cmd.resp_type = MMC_RESP_R1 | MMC_CMD_AC;

    switch (width) {
    case BUS_WIDTH_1:
        cmd.arg = SD_BUS_WIDTH_1;
        break;
    case BUS_WIDTH_4:
        cmd.arg = SD_BUS_WIDTH_4;
        break;
    default:
        return -EINVAL;
    }
    return mmc_wait_for_app_cmd(card, &cmd, MMC_CMD_RETRIES);
}

int sd_app_send_op_cond(struct mmc_card *card, uint32_t ocr, uint32_t *rocr)
{
    struct mmc_cmd cmd = {0};
    int i = 100, err = 0;

    cmd.cmd_code = SD_ACMD_OP_COND;
    cmd.arg = ocr;
    cmd.resp_type = MMC_RESP_R3 | MMC_CMD_BCR;

    do {
        err = mmc_wait_for_app_cmd(card, &cmd, MMC_CMD_RETRIES);
        if (err)
            break;

        /* if probing, just single pass */
        if (ocr == 0)
            break;
        /* wait until init complete */
        if (cmd.resp[0] & MMC_CARD_BUSY)
            break;

        err = -ETIMEDOUT;

        mmc_delay_ms(20);
    } while (i--);
    if (rocr)
        *rocr = cmd.resp[0];

    return err;
}

int sd_send_if_cond(struct mmc_card *card, uint32_t ocr)
{
    struct mmc_cmd cmd = {0};
    int err;

    cmd.cmd_code = SD_SEND_IF_COND;
    cmd.arg = ((ocr & 0xFF8000) != 0) << 8 | 0xAA;
    cmd.resp_type = MMC_RESP_SPI_R7 | MMC_RESP_R7 | MMC_CMD_BCR;

    err = mmc_wait_for_cmd(card->host, &cmd, 0);
    if (err) {
        return err;
    }
    if ((cmd.resp[0] & 0xFF) != 0xAA) {
        return -EIO;
    }
    return 0;
}

int sd_send_relative_addr(struct mmc_card *card, uint32_t *rca)
{
    int err;
    struct mmc_cmd cmd = {0};

    cmd.cmd_code = SD_SEND_RELATIVE_ADDR;
    cmd.arg = 0;
    cmd.resp_type = MMC_RESP_R6 | MMC_CMD_BCR;

    err = mmc_wait_for_cmd(card->host, &cmd, MMC_CMD_RETRIES);
    if (err)
        return err;

    *rca = cmd.resp[0] >> 16;

    return 0;
}

int sd_app_send_scr(struct mmc_card *card, uint32_t *scr)
{
    int err;
    struct mmc_request mrq = {0};
    struct mmc_cmd cmd = {0};
    struct mmc_data data = {0};
    struct scatterlist sg;
    void *data_buf;
    void *data_buf_aligned;

    err = mmc_app_cmd(card);
    if (err)
        return err;
    /* XXX: cache aligned */
    data_buf = memalign(CACHE_ALIGNED_SIZE, ALIGN(sizeof(card->card_reg.scr_raw),CACHE_ALIGNED_SIZE));
    if (data_buf == NULL) {
        mmc_err("No memory for data_buf.\n");
        return -ENOMEM;
    }
    data_buf_aligned = data_buf;
    sg_init_one(&sg, data_buf_aligned, 8);

    cmd.cmd_code = SD_ACMD_SEND_SCR;
    cmd.arg = 0;
    cmd.resp_type = MMC_RESP_R1 | MMC_CMD_ADTC;
    mrq.cmd = &cmd;

    data.blocksz = 8;
    data.blocks = 1;
    data.data_flags = MMC_DATA_READ;
    data.sg = &sg;
    data.sg_len = 1;
    mrq.data = &data;
    data.data_flags &= ~(MMC_CMD_STOP);

    mmc_set_data_timeout(&data, card);
    mmc_wait_for_req(card->host, &mrq);
    memcpy(scr, data_buf_aligned, sizeof(card->card_reg.scr_raw));
    mmc_trace(MMC_TRACE_DEBUG, "scr: 0x%x, 0x%x", scr[0], scr[1]);
    free(data_buf);

    if (cmd.err)
        return cmd.err;
    else if (data.err)
        return data.err;
    else {
        scr[0] = le32_to_cpu(scr[0]);
        scr[1] = le32_to_cpu(scr[1]);
        return 0;
    }
}

int sd_switch_func(struct mmc_card *card, int mode, int group,
    unsigned char value, unsigned char *resp)
{
    struct mmc_request mrq = {0};
    struct mmc_cmd cmd = {0};
    struct mmc_data data = {0};
    struct scatterlist sg;
    unsigned char *data_buf;
    unsigned char *data_buf_aligned;

    /* XXX: cache aligned */
    data_buf = memalign(CACHE_ALIGNED_SIZE, ALIGN(64,CACHE_ALIGNED_SIZE));
    if (data_buf == NULL) {
        mmc_err("No memory for data_buf.\n");
        return -ENOMEM;
    }
    data_buf_aligned = data_buf;
    memcpy(data_buf_aligned, resp, 64);
    sg_init_one(&sg, data_buf_aligned, 64);

    //mode = !!mode;
    value &= 0xF;

    cmd.cmd_code = SD_SWITCH_FUNC;
    cmd.arg = mode << 31 | 0x00FFFFFF;
    cmd.arg &= ~(0xF << (group * 4));
    cmd.arg |= value << (group * 4);
    cmd.resp_type = MMC_RESP_SPI_R1 | MMC_RESP_R1 | MMC_CMD_ADTC;
    mrq.cmd = &cmd;

    data.blocksz = 64;
    data.blocks = 1;
    data.data_flags = MMC_DATA_READ;
    data.sg = &sg;
    data.sg_len = 1;
    data.data_flags &= ~(MMC_CMD_STOP);
    mrq.data = &data;

    mmc_set_data_timeout(&data, card);

    mmc_wait_for_req(card->host, &mrq);

    memcpy(resp, data_buf_aligned, 64);

    free(data_buf);

    if (cmd.err)
        return cmd.err;
    if (data.err)
        return data.err;

    return 0;
}

int sd_app_sd_status(struct mmc_card *card, void *ssr)
{
    int err;
    struct mmc_request mrq = {0};
    struct mmc_cmd cmd = {0};
    struct mmc_data data = {0};
    struct scatterlist sg;
    unsigned char *data_buf;
    unsigned char *data_buf_aligned;

    err = mmc_app_cmd(card);
    if (err)
        return err;
    /* XXX: cache aligned */
    data_buf = memalign(CACHE_ALIGNED_SIZE, ALIGN(64,CACHE_ALIGNED_SIZE));
    if (!data_buf) {
        mmc_err("No memory for data_buf.\n");
        return -ENOMEM;
    }
    data_buf_aligned = data_buf;
    memcpy(data_buf_aligned, ssr, 64);
    sg_init_one(&sg, data_buf_aligned, 64);

    cmd.cmd_code = SD_ACMD_SD_STATUS;
    cmd.arg = 0;
    cmd.resp_type = MMC_RESP_SPI_R2 | MMC_RESP_R1 | MMC_CMD_ADTC;
    mrq.cmd = &cmd;

    data.blocksz = 64;
    data.blocks = 1;
    data.data_flags = MMC_DATA_READ;
    data.sg = &sg;
    data.sg_len = 1;
    data.data_flags &= ~(MMC_CMD_STOP);
    mrq.data = &data;

    mmc_set_data_timeout(&data, card);

    mmc_wait_for_req(card->host, &mrq);

    memcpy(ssr, data_buf_aligned, 64);
    free(data_buf);

    if (cmd.err)
        return cmd.err;
    if (data.err)
        return data.err;

    return 0;
}

int sd_signal_volt_switch(struct mmc_card *card)
{
    struct mmc_cmd cmd = {0};
    int err = 0;
    mmc_assert(card);

    cmd.cmd_code = SD_SWITCH_VOLTAGE;
    cmd.arg = 0;
    cmd.resp_type = MMC_RESP_R1 | MMC_CMD_AC;

    err = mmc_wait_for_cmd(card->host, &cmd, 0);
    if (err)
        return err;

    if (!is_mmc_host_spi(card->host) && (cmd.resp[0] & RSP_R1_ERROR))
        return -EIO;

    return 0;
}

/* end of file sd_ops.c*/
