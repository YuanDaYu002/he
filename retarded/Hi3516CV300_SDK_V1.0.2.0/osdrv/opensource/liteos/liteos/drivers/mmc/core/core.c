/*cort.c begin*/

#include "mmc_core.h"

/* for sd/mmc card*/
const unsigned int tran_exp[] = {
        10000, 100000, 1000000, 10000000, 0, 0, 0, 0
};
const unsigned char tran_mant[] = {
        0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80
};

const unsigned int tacc_exp[] = {
        1, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
};

const unsigned int tacc_mant[] = {
        0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80
};

/* *
 * * ffs - find first bit set
 * * @x: the word to search
 * *
 * * This is defined the same way as
 * * the libc and compiler builtin ffs routines, therefore
 * * differs in spirit from the above ffz (man ffs).
 * */
static int ffs(int x)
{
    int r = 1;

    if (!x)
        return 0;
    if (!(x & 0xffff)) {
        x >>= 16;
        r += 16;
    }
    if (!(x & 0xff)) {
        x >>= 8;
        r += 8;
    }
    if (!(x & 0xf)) {
        x >>= 4;
        r += 4;
    }
    if (!(x & 3)) {
        x >>= 2;
        r += 2;
    }
    if (!(x & 1)) {
        //x >>= 1;
        r += 1;
    }
    return r;
}

int mmc_can_erase(struct mmc_card* card)
{
    if ((card->card_reg.csd.cmdclass & CCC_ERASE)
            && card->erase_size) {
        return 1;
    }
    return 0;
}

int mmc_can_discard(struct mmc_card *card)
{
    /* v4.5 or later */
    //if (card->ext_csd.feature_support & MMC_DISCARD_FEATURE)
    //    return 1;
    return 0;
}

int mmc_can_trim(struct mmc_card *card)
{
    /* v4.5 or later */
    //if (card->ext_csd.sec_feature_support & E_CSD_SEC_GB_CL_EN)
    return 0;
}

static unsigned int mmc_align_erase_size(struct mmc_card *card,
        unsigned int *from,
        unsigned int *to,
        unsigned int nr)
{
    unsigned int from_new = *from, nr_new = nr, rem;

    rem = from_new % card->erase_size;
    if (rem) {
        rem = card->erase_size - rem;
        from_new += rem;
        if (nr_new > rem)
            nr_new -= rem;
        else
            return 0;
    }

    rem = nr_new % card->erase_size;
    if (rem)
        nr_new -= rem;

    if (nr_new == 0)
        return 0;

    *to = from_new + nr_new;
    *from = from_new;

    return nr_new;
}


static int mmc_do_erase(struct mmc_card *card, unsigned int from,
        unsigned int to, unsigned int arg)
{
    struct mmc_cmd cmd = {0};
    unsigned int qty = 0, busy_timeout = 6000;
    bool use_r1b_resp = false;
    unsigned long timeout;
    int err = 0;

    if (!is_card_blkaddr(card)) {
        from <<= 9;
        to <<= 9;
    }

    if (is_card_sd(card))
        cmd.cmd_code = SD_ERASE_WR_BLK_START;
    else
        cmd.cmd_code = ERASE_GROUP_START;
    cmd.arg = from;
    cmd.resp_type = MMC_RESP_R1 | MMC_CMD_AC;
    err = mmc_wait_for_cmd(card->host, &cmd, 0);
    if (err) {
        mmc_err("erase: start error\n");
        err = -EIO;
        goto out;
    }

    memset(&cmd, 0, sizeof(struct mmc_cmd));
    if (is_card_sd(card))
        cmd.cmd_code = SD_ERASE_WR_BLK_END;
    else
        cmd.cmd_code = ERASE_GROUP_END;

    cmd.arg = to;
    cmd.resp_type = MMC_RESP_R1 | MMC_CMD_AC;
    err = mmc_wait_for_cmd(card->host, &cmd, 0);
    if (err) {
        mmc_err("erase: end error\n");
        err = -EIO;
        goto out;
    }

    memset(&cmd, 0, sizeof(struct mmc_cmd));
    cmd.cmd_code = MMC_ERASE;
    cmd.arg = arg;

    cmd.resp_type = MMC_RESP_R1 | MMC_CMD_AC;

    err = mmc_wait_for_cmd(card->host, &cmd, 0);
    if (err) {
        mmc_err("erase: end error, err = %d ,status = 0x%x\n",
                err, cmd.resp[0]);
        err = -EIO;
        goto out;
    }

    timeout = mmc_get_sys_ticks() + mmc_ms_to_ticks(busy_timeout);
    do {
        memset(&cmd, 0, sizeof(struct mmc_cmd));
        cmd.cmd_code = SEND_STATUS;
        cmd.arg = card->card_reg.rca << 16;
        cmd.resp_type = MMC_RESP_R1 | MMC_CMD_AC;

        err = mmc_wait_for_cmd(card->host, &cmd, 0);
        if (err || (cmd.resp[0] & 0xFDF92000)) {
            mmc_err("erase:  err = %d ,status = 0x%x\n",
                err, cmd.resp[0]);
            err = -EIO;
            goto out;
        }
        if (mmc_get_sys_ticks() >= timeout) {
            err = -EIO;
            goto out;
        }

    } while((!cmd.resp[0] & R1_READY_FOR_DATA) ||
            (R1_CURRENT_STATE(cmd.resp[0] == RSP_R1_STATE_PRG)));

out:
    return err;
}

int mmc_erase(struct mmc_card *card, unsigned int from, unsigned int nr,
        unsigned int arg)
{
    unsigned int to = from + nr;

    if (!(card->card_reg.csd.cmdclass & CCC_ERASE))
        return -EOPNOTSUPP;

    if (!card->erase_size)
        return -EOPNOTSUPP;

    if (is_card_sd(card) && arg != MMC_ERASE_ARG)
        return -EOPNOTSUPP;

    if (arg == MMC_ERASE_ARG)
        nr = mmc_align_erase_size(card, &from, &to, nr);

    if (nr == 0)
        return 0;

    if (to <= from)
        return -EINVAL;

    to -=1;

    return mmc_do_erase(card, from, to, arg);
}

unsigned int mmc_get_bits(unsigned int *bits, int start, int size)
{
    const int sz = size;
    const unsigned int mask = ((sz < 32) ? (1 << sz) : 0) - 1;
    const int off = 3 - (start / 32);
    const int shift = start & 31;
    unsigned int ret;

    ret = bits[off] >> shift;
    if (sz + shift > 32)
        ret |= bits[off - 1] << ((32 - shift) % 32);
    ret &= mask;

    return ret;
}

void mmc_start(struct mmc_card* card, struct mmc_req_entity* req_entity)
{
    struct mmc_host * host = card->host;

    mmc_acquire_card(card);
    mmc_wait_for_req(host, &req_entity->mrq);
    mmc_release_card(card);
}

void mrq_done(struct mmc_request * mrq)
{
    mmc_event_signal(&mrq->mrq_event, MMC_REQUEST_DONE);
}

int wait_card_ready(struct mmc_card *card)
{
    int err = 0;
    unsigned int status;
    int i = 100;

    do {
        err = get_card_status(card, &status);
        if (err) {
            mmc_err("get card status error\n");
            return err;
        }

        if (i >= 0) {
            i--;
            msleep(10);
        } else {
            mmc_err("wait card return to ready timeout!\n");
            return -ETIMEDOUT;
        }
    }while(!(status & R1_READY_FOR_DATA) ||
            (R1_CURRENT_STATE(status) == CARD_STATE_PRG));
    return err;
}

int sent_stop(struct mmc_card *card, int dir,
        unsigned int * stop_status)
{
    struct mmc_cmd cmd = {0};
    int err;

    cmd.cmd_code = STOP_TRANSMISSION;

    //if (!IS_REQUEST_READ(mreq->request)) {
    if (dir)
        cmd.resp_type = MMC_RESP_R1B | MMC_CMD_AC;
    else
        cmd.resp_type = MMC_RESP_R1 | MMC_CMD_AC;

    err = mmc_wait_for_cmd(card->host, &cmd, 5);
    if (err) {
        return err;
    }

    *stop_status = cmd.resp[0];

    /* No need to check card status in case of READ. */
    //if (IS_REQUEST_READ(mreq->request))
    if (!dir)
        return 0;

    //if (*stop_status & R1_ERROR) {
    //    mmc_err("pf\n");
    //}

    return wait_card_ready(card);
}
void mmc_wait_for_req(struct mmc_host *host, struct mmc_request *mrq)
{
    int ret = 0;
    mrq->cmd->err = 0;
    if (mrq->data) {
        mmc_assert(mrq->data->blocksz <= host->max_blk_size);
        mmc_assert((mrq->data->blocks * mrq->data->blocksz) <=
                host->max_request_size);

        mrq->data->err = 0;
        //if (mrq->stop) {
        if (MMC_CMD_STOPENABLE(mrq->data)) {
            //mrq->stop->err = 0;
            mrq->data->cmd_stop.err = 0;
        }
    }
    if (host->caps2.bits.caps2_speedup_enable) {
        if ((mrq->cmd->resp_type & MMC_CMD_TYPE_RW) == 0) {
            mmc_event_init(&mrq->mrq_event);
            mrq->done = mrq_done;
        }
    }
    mmc_do_request(host, mrq);
    if (host->caps2.bits.caps2_speedup_enable) {
        if ((mrq->cmd->resp_type & MMC_CMD_TYPE_RW) == 0) {
            ret = mmc_event_wait(&mrq->mrq_event, MMC_REQUEST_DONE, MMC_REQUEST_TIMEOUT);
            if (ret != MMC_REQUEST_DONE) {
                mmc_trace(5, "wait request fail! ret = %d\n",ret);
            }
            mmc_event_delete(&mrq->mrq_event);
        }
    }
}

int mmc_wait_for_cmd(struct mmc_host *host, struct mmc_cmd *cmd, int retries)
{
    struct mmc_request mrq = {0};

    WARN_ON(!host->claimed);

    memset(cmd->resp, 0, sizeof(cmd->resp));
    cmd->retries = retries;

    mrq.cmd = cmd;
    do {
        mmc_wait_for_req(host, &mrq);
        if (!cmd->err)
            break;
    } while (cmd->retries--);

    return cmd->err;
}

void mmc_set_data_timeout(struct mmc_data *data, const struct mmc_card *card)
{
    unsigned int mult = 0;
    if (is_card_sdio(card)) {
        data->timeout_ns = 1000000000;
        data->timeout_clks = 0;
        return;
    }
    mult = is_card_sd(card) ? 100 : 10;
    if (data->data_flags & MMC_DATA_WRITE)
        mult <<= (card->card_reg.csd.r2w_factor);
    data->timeout_ns = card->card_reg.csd.tacc_ns * mult;
    data->timeout_clks = card->card_reg.csd.tacc_clks * mult;

    if (is_card_sd(card)) {
        unsigned int timeout_us, limit_us;

        timeout_us = data->timeout_ns / 1000;
        if (card->iocfg.clock)
            timeout_us += data->timeout_clks * 1000 /
                (card->iocfg.clock / 1000);

        if (data->data_flags & MMC_DATA_WRITE)
            limit_us = 3000000;
        else
            limit_us = 100000;

        if (timeout_us > limit_us || is_card_blkaddr(card)) {
            data->timeout_ns = limit_us * 1000;
            data->timeout_clks = 0;
        }
    }
}

/*
 * Mask off any voltages we don't support and select
 * the lowest voltage
 */
uint32_t mmc_select_voltage(struct mmc_host *host, uint32_t ocr)
{
    uint32_t bit;

    ocr &= host->ocr_default.ocr_data;

    bit = ffs(ocr);
    if (bit) {
        bit -= 1;
        ocr &= 3 << bit;
        host->dvdd = bit;
    } else {
        ocr = 0;
    }
    return ocr;
}

/* end of file core.c*/
