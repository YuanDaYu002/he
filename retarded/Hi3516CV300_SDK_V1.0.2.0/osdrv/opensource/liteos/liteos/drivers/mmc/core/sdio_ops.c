/* sdio_ops.c begin */
#include "mmc_core.h"

int sdio_io_send_op_cond(struct mmc_card *card, uint32_t ocr, uint32_t *rocr)
{
    struct mmc_cmd cmd = {0};
    int i = 100, err = 0;

    cmd.cmd_code = SDIO_SEND_OP_COND;
    cmd.arg = ocr;
    cmd.resp_type = MMC_RESP_SPI_R4 | MMC_RESP_R4 | MMC_CMD_BCR;

    while(i)
    {
        err = mmc_wait_for_cmd(card->host, &cmd, MMC_CMD_RETRIES);
        if ( (err) || (ocr == 0) )
            break;
        if (cmd.resp[0] & MMC_CARD_BUSY)
            break;

        err = -ETIMEDOUT;

        mmc_delay_ms(10);
        i--;
    }
    if (rocr)
        *rocr = cmd.resp[0];

    return err;
}

int sdio_io_rw_direct(struct mmc_card *card, int write, uint32_t fn,
    uint32_t addr, uint8_t in, uint8_t *out)
{
    struct mmc_cmd cmd = {0};
    int err;

    cmd.cmd_code = SDIO_RW_DIRECT;
    cmd.resp_type = MMC_RESP_SPI_R5 | MMC_RESP_R5 | MMC_CMD_AC;
    cmd.arg = write ? 0x80000000 : 0x00000000;
    cmd.arg |= addr << 9;
    cmd.arg |= fn << 28;
    cmd.arg |= in;
    cmd.arg |= (write && out) ? 0x08000000 : 0x00000000;

    err = mmc_wait_for_cmd(card->host, &cmd, 0);
    if (err)
        return err;

    if (cmd.resp[0] & SDIO_R5_ERROR)
        return -EIO;
    if (cmd.resp[0] & SDIO_R5_OUT_OF_RANGE)
        return -ERANGE;
    if (cmd.resp[0] & SDIO_R5_FUNCTION_NUMBER)
        return -EINVAL;

    if (out)
        *out = cmd.resp[0] & 0xFF;

    return 0;
}

int sdio_rw_direct(struct mmc_card *card, int write, uint32_t fn,
    uint32_t addr, uint8_t in, uint8_t *out)
{
    return sdio_io_rw_direct(card, write, fn, addr, in, out);
}

int sdio_io_rw_extended(struct mmc_card *card, int write, uint32_t fn,
    uint32_t addr, int incr_addr, uint8_t *buf, uint32_t blocks, uint32_t blksz)
{
    struct mmc_data data = {0};
    struct mmc_request req = {0};
    struct mmc_cmd cmd = {0};
    struct scatterlist sg;
    static uint8_t *aligned_buf = NULL;

    /* sanity check */
    if (addr & ~0x1FFFF)
        return -EINVAL;

    req.cmd = &cmd;
    req.data = &data;

    cmd.cmd_code = SDIO_RW_EXTENDED;
    cmd.arg = write ? 0x80000000 : 0x00000000;
    cmd.arg |= incr_addr ? 0x04000000 : 0x00000000;
    cmd.arg |= fn << 28;
    cmd.arg |= addr << 9;
    cmd.resp_type = MMC_RESP_SPI_R5 | MMC_RESP_R5 | MMC_CMD_ADTC;

    if (blocks == 1 && blksz <= 512)
        cmd.arg |= (blksz == 512) ? 0 : blksz;    /* byte mode */
    else
        cmd.arg |= 0x08000000 | blocks;        /* block mode */

    data.blocks = blocks;
    data.blocksz = blksz;
    data.data_flags = write ? MMC_DATA_WRITE : MMC_DATA_READ;
    data.sg = &sg;
    data.sg_len = 1;
    data.data_flags &= ~MMC_CMD_STOP;

    mmc_acquire_card(card);
    if (aligned_buf == NULL) {
        aligned_buf = memalign(CACHE_ALIGNED_SIZE, ALIGN(32*1024, CACHE_ALIGNED_SIZE));
    }

    memcpy(aligned_buf, buf, blksz * blocks);
    sg_init_one(&sg, aligned_buf, blksz * blocks);

    mmc_set_data_timeout(&data, card);
    mmc_wait_for_req(card->host, &req);

    if (write == 0) {
        memcpy(buf, aligned_buf, blksz * blocks);
    }

    mmc_release_card(card);
    if (cmd.err)
        return cmd.err;
    if (data.err)
        return data.err;

    if (cmd.resp[0] & SDIO_R5_ERROR)
        return -EIO;
    if (cmd.resp[0] & SDIO_R5_FUNCTION_NUMBER)
        return -EINVAL;
    if (cmd.resp[0] & SDIO_R5_OUT_OF_RANGE)
        return -ERANGE;

    return 0;
}

static int cistpl_manfid_info(struct mmc_card *card, struct sdio_func *func,
        const uint8_t *buf, uint32_t size)
{
    uint16_t vendor, device;

    /* TPLMID_MANF */
    vendor = (buf[1] << 8) | buf[0];

    /* TPLMID_CARD */
    device = (buf[3] << 8) | buf[2];

    if (func) {
        func->device_id = device;
        func->manufacturer_id= vendor;
    } else {
        card->card_reg.cis.device = device;
        card->card_reg.cis.vendor = vendor;
    }

    return 0;
}

static const uint8_t speed_val[16] =
{ 0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80 };
static const uint32_t speed_unit[8] =
{ 10000, 100000, 1000000, 10000000, 0, 0, 0, 0 };


typedef int (tpl_parse_t)(struct mmc_card *, struct sdio_func *,
        const uint8_t *, uint32_t);

struct cis_tpl {
    uint8_t code;
    uint8_t min_size;
    tpl_parse_t *parse;
};


static int cis_tpl_parse(struct mmc_card *card, struct sdio_func *func,
        const char *tpl_descr,
        const struct cis_tpl *tpl, int tpl_count,
        uint8_t code,
        const uint8_t *buf, uint32_t size)
{
    int err ,i = 0;

    while(i < tpl_count)
    {
        if (tpl->code == code)
            break;
        i++;
        tpl++;
    }

    if (i < tpl_count) {
        if (size >= tpl->min_size) {
            if (tpl->parse)
                err = tpl->parse(card, func, buf, size);
            else
                err = -EILSEQ;
        } else {
            err = -EINVAL;
        }
        if (err && err != -EILSEQ && err != -ENOENT) {
            mmc_printf(MMC_PRINTF_WARN, "bad %s tuple 0x%02x (%u bytes)\n",
                    tpl_descr, code, size);
        }
    } else
        err = -ENOENT;

    return err;
}

static int cistpl_funce_common(struct mmc_card *card, struct sdio_func *func,
                   const uint8_t *buf, uint32_t size)
{
    if (func)
        return -EINVAL;

    //card->cis.blksize = buf[1] | (buf[2] << 8);
    card->card_reg.cis.blksize = (buf[2] << 8) | buf[1];
    card->card_reg.cis.max_dtr = speed_unit[buf[3] & 7] *
                speed_val[(buf[3] >> 3) & 15];

    return 0;
}

static int cistpl_funce_func(struct mmc_card *card, struct sdio_func *func,
                 const uint8_t *buf, uint32_t size)
{
    uint32_t vsn;
    uint32_t min_size;

    if (!func)
        return -EINVAL;

    vsn = func->card->card_reg.cccr.sdio_vsn;
    min_size = (vsn == SDIO_SDIO_REV_1_00) ? 28 : 42;

    if (size < min_size)
        return -EINVAL;

    func->max_blk_size = buf[12] | (buf[13] << 8);

    if (vsn > SDIO_SDIO_REV_1_00)
        func->en_timeout_ms = (buf[28] | (buf[29] << 8)) * 10;
    else
        func->en_timeout_ms = 100;

    return 0;
}

static int cistpl_funce(struct mmc_card *card, struct sdio_func *func,
            const uint8_t *buf, uint32_t size);

static const struct cis_tpl cis_tpl_funce_list[] = {
    {    0x00,    4,    cistpl_funce_common        },
    {    0x01,    0,    cistpl_funce_func        },
    {    0x04,    8,    NULL    },
};

static const struct cis_tpl cis_tpl_list[] = {
    {    0x15,    3,    NULL     },
    {    0x20,    4,    cistpl_manfid_info    },
    {    0x21,    2,    NULL    },
    {    0x22,    0,    cistpl_funce        },
};

static int cistpl_funce(struct mmc_card *card, struct sdio_func *func,
            const uint8_t *buf, uint32_t size)
{
    if (size < 1)
        return -EINVAL;

    return cis_tpl_parse(card, func, "CISTPL_FUNCE",
                 cis_tpl_funce_list,
                 ARRAY_SIZE(cis_tpl_funce_list),
                 buf[0], buf, size);
}

int sdio_read_cis(struct mmc_card *card, struct sdio_func *func)
{
    int err = 0;
    uint32_t i = 0, ptr = 0;
    uint8_t data, fnum;
    uint8_t tpl_code, tpl_link;
    struct sdio_func_tuple *this, **prev;

    while(i < 3)
    {
        if (func)
            fnum = func->func_num;
        else
            fnum = 0;
        err = sdio_rw_direct(card, 0, 0,
                SDIO_FBR_BASE(fnum) + SDIO_FBR_P_CIS + i, 0, &data);
        if (err)
            return err;
        ptr |= data << (i * 8);
        i++;
    }
    if (func)
        prev = &func->tuple_link;
    else
        prev = &card->tuple_link;

    do {
        err = sdio_rw_direct(card, 0, 0, ptr++, 0, &tpl_code);
        if (err)
            break;
        if (tpl_code == 0xff)
            break;
        if (tpl_code == 0x00)
            continue;
        err = sdio_rw_direct(card, 0, 0, ptr++, 0, &tpl_link);
        if (err)
            break;
        if (tpl_link == 0xff)
            break;
        this = malloc(sizeof(*this) + tpl_link);
        if (!this)
            return -ENOMEM;
        for (i = 0; i < tpl_link; i++) {
            err = sdio_rw_direct(card, 0, 0,
                    ptr + i, 0, &this->data[i]);
            if (err)
                break;
        }
        if (err) {
            free(this);
            break;
        }
        err = cis_tpl_parse(card, func, "CIS",
                cis_tpl_list, ARRAY_SIZE(cis_tpl_list),
                tpl_code, this->data, tpl_link);
        if (err == -EILSEQ || err == -ENOENT) {
            this->size = tpl_link;
            this->code = tpl_code;
            this->next = NULL;
            *prev = this;
            prev = &this->next;

            if (err == -ENOENT) {
            }

            err = 0;
        } else {
            free(this);
            this = NULL;
        }

        ptr += tpl_link;
    } while (!err);
    if (func)
        *prev = card->tuple_link;

    return err;
}

int sdio_read_func_cis(struct sdio_func *func)
{
    int err;

    err = sdio_read_cis(func->card, func);
    if (err)
        return err;
    if (func->manufacturer_id == 0) {
        func->device_id = func->card->card_reg.cis.device;
        func->manufacturer_id = func->card->card_reg.cis.vendor;
    }

    return 0;
}

/* sdio_ops.c end */
