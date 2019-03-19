/*mmc_ops.c begin*/
#include "mmc_core.h"

int mmc_app_cmd(struct mmc_card *card)
{
    int err = 0;
    struct mmc_cmd cmd = {0};

    cmd.cmd_code = APP_CMD;

    if (!is_card_unknow(card)) {
        cmd.arg = (card->card_reg.rca) << 16;
        cmd.resp_type = MMC_RESP_R1 | MMC_CMD_AC;
    } else {
        cmd.arg = 0;
        cmd.resp_type = MMC_RESP_R1 | MMC_CMD_BCR;
    }

    err = mmc_wait_for_cmd(card->host, &cmd, 0);
    if (err)
        return err;

    if (!(cmd.resp[0] & RSP_R1_APP_CMD))
        return -EOPNOTSUPP;

    return 0;
}

int mmc_select_card(struct mmc_card *card)
{
    if (card == NULL)
        return -EINVAL;
    struct mmc_cmd cmd = {0};

    cmd.cmd_code = SELECT_CARD;
    cmd.arg = card->card_reg.rca << 16;
    cmd.resp_type = MMC_RESP_R1 | MMC_CMD_AC;

    return mmc_wait_for_cmd(card->host, &cmd, MMC_CMD_RETRIES);
}

void mmc_idle_card(struct mmc_card *card)
{
    struct mmc_cmd cmd = {0};

    cmd.cmd_code = GO_IDLE_STATE;
    cmd.arg = 0;
    cmd.resp_type = MMC_RESP_SPI_R1 | MMC_RESP_NONE | MMC_CMD_BC;
    /* idle cmd, needn't check return value */
    (void)mmc_wait_for_cmd(card->host, &cmd, 0);
    mmc_delay_ms(1);
}

int mmc_send_op_cond(struct mmc_card *card, uint32_t ocr, uint32_t *rocr)
{
    struct mmc_cmd cmd = {0};
    int i, err = 0;

    cmd.cmd_code = SEND_OP_COND;
    cmd.arg = ocr;
    cmd.resp_type = MMC_RESP_SPI_R1 | MMC_RESP_R3 | MMC_CMD_BCR;

    for (i = 0; i < 100; i++) {
        err = mmc_wait_for_cmd(card->host, &cmd, 0);
        if (err)
            break;

        if ((ocr == 0) || (cmd.resp[0] & MMC_CARD_BUSY))
            break;

        err = -ETIMEDOUT;
        mmc_delay_ms(10);
    }

    if (rocr && !err)
        *rocr = cmd.resp[0];

    return err;
}

int mmc_all_send_cid(struct mmc_card *card, uint32_t *cid)
{
    struct mmc_cmd cmd = {0};
    int err = 0;

    cmd.cmd_code = ALL_SEND_CID;
    cmd.arg = 0;
    cmd.resp_type = MMC_RESP_R2 | MMC_CMD_BCR;

    err = mmc_wait_for_cmd(card->host, &cmd, MMC_CMD_RETRIES);
    memcpy(cid, cmd.resp, sizeof(uint32_t) * 4);

    return err;
}

int mmc_set_relative_addr(struct mmc_card *card)
{
    struct mmc_cmd cmd = {0};
    int err = 0;

    cmd.cmd_code = SET_RELATIVE_ADDR;
    cmd.arg = card->card_reg.rca << 16;
    cmd.resp_type = MMC_RESP_R1 | MMC_CMD_AC;

    err = mmc_wait_for_cmd(card->host, &cmd, MMC_CMD_RETRIES);

    return err;
}

int mmc_send_csd(struct mmc_card *card, uint32_t *csd)
{
    int err = 0;
    struct mmc_cmd cmd = {0};

    cmd.cmd_code = SEND_CSD;
    cmd.arg = (card->card_reg.rca << 16);
    cmd.resp_type = MMC_RESP_R2 | MMC_CMD_AC;
    err = mmc_wait_for_cmd(card->host, &cmd, MMC_CMD_RETRIES);
    memcpy(csd, cmd.resp, sizeof(uint32_t) * 4);

    return err;
}

int mmc_send_cid(struct mmc_host *host, uint32_t *cid)
{
    int err = 0;
    struct mmc_cmd cmd = {0};

    if (!host->card_cur)
        return -EINVAL;
    cmd.cmd_code = SEND_CID;
    cmd.arg = (host->card_cur->card_reg.rca << 16);
    cmd.resp_type = MMC_RESP_R2 | MMC_CMD_AC;
    err = mmc_wait_for_cmd(host, &cmd, MMC_CMD_RETRIES);
    memcpy(cid, cmd.resp, sizeof(uint32_t) * 4);

    return err;
}

int mmc_send_ext_csd(struct mmc_card *card, uint8_t *ext_csd)
{
    uint32_t len = 512;
    struct mmc_request mrq = {0};
    struct mmc_cmd cmd = {0};
    struct mmc_data data = {0};
    struct scatterlist sg;
    void *data_buf;
    void *data_buf_aligned;

    data_buf = memalign(CACHE_ALIGNED_SIZE, ALIGN(len,CACHE_ALIGNED_SIZE));
    if (data_buf == NULL)
        return -ENOMEM;

    data_buf_aligned = data_buf;
    sg_init_one(&sg, data_buf_aligned, len);

    cmd.cmd_code = SEND_EXT_CSD;
    cmd.arg = 0;
    cmd.resp_type = MMC_RESP_SPI_R1 | MMC_RESP_R1 | MMC_CMD_ADTC;
    mrq.cmd = &cmd;

    data.blocksz = len;
    data.blocks = 1;
    data.data_flags = MMC_DATA_READ;
    data.sg = &sg;
    data.sg_len = 1;
    mrq.data = &data;
    data.data_flags &= ~(MMC_CMD_STOP);

    mmc_set_data_timeout(&data, card);
    mmc_wait_for_req(card->host, &mrq);
    memcpy(ext_csd, data_buf_aligned, len);

    free(data_buf);

    if (cmd.err)
        return cmd.err;
    if (data.err)
        return data.err;
    return 0;
}

int mmc_switch(struct mmc_card *card,
        uint8_t set,
        uint8_t index,
        uint8_t value,
        uint32_t timeout_ms)
{
    int err;
    struct mmc_cmd cmd = {0};
    uint32_t status;

    cmd.cmd_code = SWITCH_FUNC;
    cmd.arg = (SWITCH_FUNC_WRITE_BYTE << 24)
            | (index << 16)
            | (value << 8)
            | set;
    cmd.resp_type = MMC_RESP_SPI_R1B | MMC_RESP_R1B | MMC_CMD_AC;
    //cmd.timeout_ms = timeout_ms;

    err = mmc_wait_for_cmd(card->host, &cmd, MMC_CMD_RETRIES);
    if (err)
        return err;

    /* Must check status to be sure of no errors */
    do {
        err = mmc_send_status(card, &status);
        if (err)
            return err;
        if (card->host->caps.bits.cap_wait_while_busy)
            break;
    } while (RSP_R1_CURRENT_STATE(status) == RSP_R1_STATE_PRG);

    if (status & 0xFDFFA000)
        mmc_printf(MMC_PRINTF_WARN, "unexpected status %#x after switch",
                status);

    if (status & RSP_R1_SWITCH_ERROR)
        return -EBADMSG;

    return 0;
}

int mmc_send_status(struct mmc_card *card, uint32_t *status)
{
    int err;
    struct mmc_cmd cmd = {0};

    cmd.cmd_code = SEND_STATUS;
    cmd.arg = card->card_reg.rca << 16;
    cmd.resp_type = MMC_RESP_SPI_R2 | MMC_RESP_R1 | MMC_CMD_AC;

    err = mmc_wait_for_cmd(card->host, &cmd, MMC_CMD_RETRIES);

    if (status)
        *status = cmd.resp[0];

    return err;
}

/* end of file mmc_ops.c*/
