/*sdio.c begin*/
#include "mmc_core.h"

static int sdio_read_fbr(struct sdio_func *func)
{
    int err = 0;
    uint8_t data;
    uint32_t fbr_addr = 0;

    fbr_addr = SDIO_FBR_BASE(func->func_num) + SDIO_FBR_STD_FUNC_IF_C;
    err = sdio_rw_direct(func->card, 0, 0, fbr_addr, 0, &data);
    if (err)
        goto out;

    data &= 0x0f;

    if (data == 0x0f) {
        fbr_addr = SDIO_FBR_BASE(func->func_num) + SDIO_FBR_STD_IF_EXT;
        err = sdio_rw_direct(func->card, 0, 0, fbr_addr, 0, &data);
        if (err)
            goto out;
    }

    func->func_class = data;

out:
    return err;
}

struct sdio_func *sdio_alloc_func(struct mmc_card *card)
{
    struct sdio_func *func = NULL;

    func = malloc(sizeof(struct sdio_func));
    if (!func)
        return NULL;

    memset(func, 0, sizeof(struct sdio_func));
    func->card = card;

    return func;
}

static int sdio_init_func(struct mmc_card *card, uint32_t fn)
{
    int err = 0;
    struct sdio_func *func;

    func = sdio_alloc_func(card);
    if (!func)
        return -ENOMEM;
    func->func_num = fn;

    if (!(is_card_not_std_sdio(card))) {
        err = sdio_read_fbr(func);
        if (err)
            goto fail;

        err = sdio_read_func_cis(func);
        if (err)
            goto fail;
    } else {
        func->manufacturer_id = func->card->card_reg.cis.vendor;
        func->max_blk_size = func->card->card_reg.cis.blksize;
        func->device_id = func->card->card_reg.cis.device;
    }

    card->sdio_funcs[fn - 1] = func;

    return 0;

fail:
    if (func)
        free(func);
    return err;
}

static int sdio_read_cccr(struct mmc_card *card)
{
    int err;
    int vsn;
    unsigned char data;

    memset(&card->card_reg.cccr, 0, sizeof(struct sdio_cccr));

    err = sdio_rw_direct(card, 0, 0, SDIO_CCCR, 0, &data);
    if (err)
        goto out;

    vsn = data & 0x0f;

    if (vsn > SDIO_CCCR_REV_3_00) {
        mmc_printf(MMC_PRINTF_WARN, "unknow CCCR structure version %d\n",
            vsn);
        return -EINVAL;
    }
    card->card_reg.cccr.sdio_vsn = (data & 0xf0) >> 4;

    err = sdio_rw_direct(card, 0, 0, SDIO_CCCR_CAPS, 0, &data);
    if (err)
        goto out;

    if (data & SDIO_CCCR_CAP_4BLS)
        card->card_reg.cccr.wide_bus = 1;
    if (data & SDIO_CCCR_CAP_LSC)
        card->card_reg.cccr.low_speed = 1;
    if (data & SDIO_CCCR_CAP_SMB)
        card->card_reg.cccr.multi_block = 1;

    if (vsn >= SDIO_CCCR_REV_1_10) {
        err = sdio_rw_direct(card, 0, 0, SDIO_CCCR_POWER_CTL, 0, &data);
        if (err)
            goto out;

        if (data & SDIO_POWER_SMPC)
            card->card_reg.cccr.high_power = 1;
    }

    if (vsn >= SDIO_CCCR_REV_2_00) {
        err = sdio_rw_direct(card, 0, 0, SDIO_CCCR_SPEED_SEL, 0, &data);
        if (err)
            goto out;

        if (data & SDIO_SPEED_SHS)
            card->card_reg.cccr.high_speed = 1;
    }

out:
    return err;
}

static int sdio_enable_wide(struct mmc_card *card)
{
    int err;
    unsigned char ctrl;

    if (!(card->host->caps.bits.cap_4_bit))
        return 0;

    if (card->card_reg.cccr.low_speed && !card->card_reg.cccr.wide_bus)
        return 0;

    err = sdio_rw_direct(card, 0, 0, SDIO_CCCR_BUS_IF_C, 0, &ctrl);
    if (err)
        return err;

    ctrl |= SDIO_CCCR_WIDTH_4BIT;

    err = sdio_rw_direct(card, 1, 0, SDIO_CCCR_BUS_IF_C, ctrl, NULL);
    if (err)
        return err;

    return 1;
}

static int sdio_disable_cd(struct mmc_card *card)
{
    int err;
    unsigned char ctrl;

    err = sdio_rw_direct(card, 0, 0, SDIO_CCCR_BUS_IF_C, 0, &ctrl);
    if (err)
        return err;

    ctrl |= SDIO_CCCR_CD_DISABLE;

    return sdio_rw_direct(card, 1, 0, SDIO_CCCR_BUS_IF_C, ctrl, NULL);
}

static int sdio_enable_4bit_bus(struct mmc_card *card)
{
    int err = 0;

    if (is_card_sdio(card))
        return sdio_enable_wide(card);

    if ((card->host->caps.bits.cap_4_bit) &&
            (card->card_reg.scr.bus_widths & SD_SCR_BUS_WIDTH_4)) {
        err = sd_app_set_bus_width(card, BUS_WIDTH_4);
        if (err)
            return err;
    } else
        return 0;
    err = sdio_enable_wide(card);
    if (err <= 0)
        sd_app_set_bus_width(card, BUS_WIDTH_1);
    return err;
}

static int sdio_switch_hs(struct mmc_card *card, int enable)
{
    int err;
    unsigned char speed;

    if (!(card->host->caps.bits.cap_sd_highspeed))
        return 0;

    if (!card->card_reg.cccr.high_speed)
        return 0;

    err = sdio_rw_direct(card, 0, 0, SDIO_CCCR_SPEED_SEL, 0, &speed);
    if (err)
        return err;

    if (enable)
        speed |= SDIO_SPEED_EHS;
    else
        speed &= ~SDIO_SPEED_EHS;

    err = sdio_rw_direct(card, 1, 0, SDIO_CCCR_SPEED_SEL, speed, NULL);
    if (err)
        return err;

    return 1;
}

static int sdio_enable_hs(struct mmc_card *card)
{
    int err;

    err = sdio_switch_hs(card, TRUE);
    if (is_card_sdio(card) || (err <= 0) )
        return err;

    err = sd_switch_func_hs(card);
    if (err <= 0)
        sdio_switch_hs(card, FALSE);

    return err;
}

static uint32_t mmc_sdio_get_max_clock(struct mmc_card *card)
{
    uint32_t max_dtr;

    if (is_card_highspeed(card))
        max_dtr = 50000000;
    else
        max_dtr = card->card_reg.cis.max_dtr;

    if (is_card_combo(card))
        max_dtr = min(max_dtr, mmc_sd_get_max_clock(card));

    return max_dtr;
}

static void mmc_sdio_remove(struct mmc_card *card)
{
    int i = 0;

    while(i < card->sdio_func_num) {
        if (card->sdio_funcs[i]) {
            free(card->sdio_funcs[i]);
            card->sdio_funcs[i] = NULL;
        }
        i++;
    }
}

int sdio_reset(struct mmc_card *card)
{
    int err = 0;
    uint8_t abort = 0;
    err = sdio_io_rw_direct(card, 0, 0, SDIO_CCCR_ASx, 0, &abort);
    if (err)
        abort = 0x08;
    else
        abort |= 0x08;
    err = sdio_io_rw_direct(card, 1, 0, SDIO_CCCR_ASx, abort, NULL);
    return err;
}

int sdio_reset_comm(struct mmc_card *card)
{
    struct mmc_host *host = card->host;
    uint32_t ocr;
    int err;
    mmc_idle_card(card);
    mmc_set_clock(host, host->freq_min);
    err = sdio_io_send_op_cond(card, 0, &ocr);
    if (err)
        goto err;
    err = sdio_io_send_op_cond(card, card->card_reg.ocr.ocr_data, &ocr);
    if (err)
        goto err;
    err = sd_send_relative_addr(card, &card->card_reg.rca);
    if (err)
        goto err;
    err = mmc_select_card(card);
    if (err)
        goto err;
    err = sdio_enable_hs(card);
    if (err > 0)
        sd_go_highspeed(card);
    else if (err)
        goto err;
    mmc_set_clock(host, mmc_sdio_get_max_clock(card));
    err = sdio_enable_4bit_bus(card);
    if (err > 0)
        mmc_set_bus_width(host, BUS_WIDTH_4);
    else if (err)
        goto err;
    return 0;
err:
    mmc_printf(MMC_PRINTF_ERR, "reset SDIO comm,err:(%d)\n",
            err);
    return err;
}


int mmc_assume_sdio(struct mmc_card *card)
{
    int err, i, func_num;
    union mmc_ocr ocr;
    union mmc_ocr ocr_cid;
    struct mmc_host *host;
    mmc_assert(card);

    host = card->host;
    err = sdio_io_send_op_cond(card, 0, &ocr.ocr_data);
    if (err)
        return err;
    if (ocr.ocr_data & 0x7F) {
        mmc_printf(MMC_PRINTF_WARN,  "card claim to support voltages "
                "below the defined range. be ignored.\n");
        ocr.ocr_data &= ~0x7F;
    }

    card->card_reg.ocr.ocr_data = mmc_select_voltage(host, ocr.ocr_data);

    err = sdio_io_send_op_cond(card, card->card_reg.ocr.ocr_data, &ocr.ocr_data);
    if (err)
        goto err;

    /* identify card */
    ocr_cid.ocr_data = (card->card_reg.ocr.ocr_data) & (ocr.ocr_data);
    if ((sd_get_cid(card, ocr_cid, (uint32_t *)card->card_reg.cid_raw, NULL) == 0 )
            && (ocr.ocr_data & SDIO_R4_MEMORY_PRESENT))
        card->type = CARD_TYPE_COMBO;
    else
        card->type = CARD_TYPE_SDIO;

    err = sd_send_relative_addr(card, &card->card_reg.rca);
    if (err)
        goto err;

    if (is_card_combo(card)) {
        err = sd_get_csd(card);
        if (err)
            goto err;
        sd_decode_cid(card);
    }

    err = mmc_select_card(card);
    if (err)
        goto err;

    if (is_card_not_std_sdio(card)) {
        card->iocfg.clock = card->card_reg.cis.max_dtr;
        mmc_set_clock(host, card->iocfg.clock);

        if(card->card_reg.cccr.high_speed) {
            set_card_highspeed(card);
        }
    } else {
        err = sdio_read_cccr(card);
        if (err)
            goto err;
        err = sdio_read_cis(card, NULL);
        if (err)
            goto err;
        if (is_card_combo(card)) {
            err = sd_setup_card(card);
            if (err) {
                mmc_idle_card(card);
                card->type = CARD_TYPE_SDIO;
            }
        }
        err = sdio_disable_cd(card);
        if (err)
            goto err;

        err = sdio_enable_hs(card);
        if (err > 0)
            sd_go_highspeed(card);
        else if (err)
            goto err;

        card->iocfg.clock = mmc_sdio_get_max_clock(card);
        mmc_set_clock(host, card->iocfg.clock);

        err = sdio_enable_4bit_bus(card);
        if (err > 0) {
            card->iocfg.bus_width = BUS_WIDTH_4;
            mmc_set_bus_width(card->host, card->iocfg.bus_width);
        }
        else if (err)
            goto err;
    }
    func_num = (ocr.ocr_data >> 28) & 7;
    card->sdio_func_num = 0;
    for (i = 0; i < func_num; i++, card->sdio_func_num++) {
        err = sdio_init_func(card, i + 1);
        if (err)
            goto remove_func;
    }
    err = mmc_add_card(card);
    if (err)
        goto remove_func;
    return 0;

remove_func:
    mmc_sdio_remove(card);
err:
    mmc_reset_card(card);
    mmc_printf(MMC_PRINTF_ERR, ": error %d initializing SDIO card\n",
            err);
    return err;
}

/* sdio.c */
