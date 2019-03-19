/*sd.c begin*/
#include "mmc_core.h"

extern const unsigned int tran_exp[];
extern const unsigned char tran_mant[];
extern const unsigned int tacc_exp[];
extern const unsigned int tacc_mant[];
void sd_decode_cid(struct mmc_card *card)
{
    int i;
    uint32_t *cid_raw = card->card_reg.cid_raw;

    memset(&card->card_reg.cid, 0, sizeof(struct mmc_cid));

    card->card_reg.cid.mid = mmc_get_bits(cid_raw, 120, 8);
    card->card_reg.cid.oid = mmc_get_bits(cid_raw, 104, 16);
    for (i = 0; i < 5; i++)
        card->card_reg.cid.pnm[i] = mmc_get_bits(cid_raw, 96 - i * 8, 8);
    card->card_reg.cid.pnm[5] = 0;
    card->card_reg.cid.hwrev = mmc_get_bits(cid_raw, 60, 4);
    card->card_reg.cid.fwrev = mmc_get_bits(cid_raw, 56, 4);
    card->card_reg.cid.serial = mmc_get_bits(cid_raw, 24, 32);
    card->card_reg.cid.year = mmc_get_bits(cid_raw, 12, 8);
    card->card_reg.cid.year += 2000;
    card->card_reg.cid.month = mmc_get_bits(cid_raw, 8, 4);
}

static int sd_decode_csd(struct mmc_card *card)
{
    struct mmc_csd *csd = &card->card_reg.csd;
    unsigned int e, m, csd_struct;
    uint32_t *csd_raw = card->card_reg.csd_raw;

    csd_struct = mmc_get_bits(csd_raw, 126, 2);
    if (0 == csd_struct) {
        m = mmc_get_bits(csd_raw, 115, 4);
        e = mmc_get_bits(csd_raw, 112, 3);
        csd->tacc_ns = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
        csd->tacc_clks = mmc_get_bits(csd_raw, 104, 8) * 100;

        m = mmc_get_bits(csd_raw, 99, 4);
        e = mmc_get_bits(csd_raw, 96, 3);
        csd->max_dtr = tran_exp[e] * tran_mant[m];
        csd->cmdclass = mmc_get_bits(csd_raw, 84, 12);

        m = mmc_get_bits(csd_raw, 62, 12);
        e = mmc_get_bits(csd_raw, 47, 3);
        csd->capacity = (1 + m) << (e + 2);

        csd->read_blkbits = mmc_get_bits(csd_raw, 80, 4);
        csd->read_partial = mmc_get_bits(csd_raw, 79, 1);
        csd->write_misalign = mmc_get_bits(csd_raw, 78, 1);
        csd->read_misalign = mmc_get_bits(csd_raw, 77, 1);

        if (mmc_get_bits(csd_raw, 46, 1)) {
            csd->erase_size = 1;
        } else if (csd->write_blkbits >= 9) {
            csd->erase_size = mmc_get_bits(csd_raw, 39, 7) + 1;
            csd->erase_size <<= csd->write_blkbits - 9;
        }

        csd->r2w_factor = mmc_get_bits(csd_raw, 26, 3);
        csd->write_blkbits = mmc_get_bits(csd_raw, 22, 4);
        csd->write_partial = mmc_get_bits(csd_raw, 21, 1);
    } else if (1 == csd_struct) {
        set_card_blockaddr(card);
        csd->tacc_ns = 0;
        csd->tacc_clks = 0;
        m = mmc_get_bits(csd_raw, 99, 4);
        e = mmc_get_bits(csd_raw, 96, 3);
        csd->max_dtr = tran_exp[e] * tran_mant[m];
        csd->cmdclass = mmc_get_bits(csd_raw, 84, 12);
        csd->c_size = mmc_get_bits(csd_raw, 48, 22);

        if (csd->c_size >= 0xFFFF)
            set_card_ext_capacity(card);
        m = mmc_get_bits(csd_raw, 48, 22);
        csd->capacity     = (1 + m) << 10;

        csd->read_partial = 0;
        csd->read_blkbits = 9;
        csd->read_misalign = 0;
        csd->write_misalign = 0;
        csd->write_blkbits = 9;
        csd->write_partial = 0;
        csd->r2w_factor = 4; /* Unused */
        csd->erase_size = 1;
    } else {
        mmc_printf(MMC_PRINTF_ERR, "unrecognised CSD structure version %d\n",
                csd_struct);
        return -EINVAL;
    }

    card->erase_size = csd->erase_size;
    return 0;
}

static int sd_decode_scr(struct mmc_card *card)
{
    struct sd_scr *scr = &card->card_reg.scr;
    unsigned int scr_struct;
    unsigned int resp[4];

    resp[3] = card->card_reg.scr_raw[1];
    resp[2] = card->card_reg.scr_raw[0];

    scr_struct = mmc_get_bits(resp, 60, 4);
    if (scr_struct != 0) {
        mmc_printf(MMC_PRINTF_ERR, "unknown SCR structure version %d\n",
                scr_struct);
        return -EINVAL;
    }

    scr->sda_vsn = mmc_get_bits(resp, 56, 4);
    scr->bus_widths = mmc_get_bits(resp, 48, 4);
    if (scr->sda_vsn == SD_SCR_SPEC_VER_2)
        scr->sda_spec3 = mmc_get_bits(resp, 47, 1);
    if (scr->sda_spec3)
        scr->cmds = mmc_get_bits(resp, 32, 2);

    mmc_printf(MMC_PRINTF_INFO, "scr");
    mmc_printf(MMC_PRINTF_INFO, "----------------------------------------------");
    mmc_printf(MMC_PRINTF_INFO, "    sda_vsn: 0x%x", scr->sda_vsn);
    mmc_printf(MMC_PRINTF_INFO, "    sda_spec3: 0x%x", scr->sda_spec3);
    mmc_printf(MMC_PRINTF_INFO, "    bus_widths: 0x%x", scr->bus_widths);
    mmc_printf(MMC_PRINTF_INFO, "    cmds: 0x%x\n", scr->cmds);

    return 0;
}

static int mmc_read_ssr(struct mmc_card *card)
{
    unsigned int au, es, et, eo;
    int err, i;
    unsigned int *ssr;

    if (!(card->card_reg.csd.cmdclass & CCC_APP_SPEC)) {
        mmc_printf(MMC_PRINTF_WARN, "card lacks mandatory SD Status "
                "function.\n");
        return 0;
    }

    ssr = malloc(64);
    if (!ssr) {
        mmc_err("No memory for ssr.\n");
        return -ENOMEM;
    }
    err = sd_app_sd_status(card, ssr);
    if (err) {
        mmc_printf(MMC_PRINTF_WARN, "problem reading SD Status "
                "register.\n");
        err = 0;
        goto err;
    }

    for (i = 0; i < 16; i++)
        ssr[i] = le32_to_cpu(ssr[i]);

    au = mmc_get_bits(ssr, 428 - 384, 4);
    if (au > 0) {
        if ((au <= 9) || card->card_reg.scr.sda_spec3) {
            card->card_reg.ssr.au_value = 1 << (au + 4);
            card->card_reg.ssr.speed_class = mmc_get_bits(ssr, 440 - 384, 8);
            card->card_reg.ssr.uhs_speed_grade = mmc_get_bits(ssr, 396 - 384, 4);
            es = mmc_get_bits(ssr, 408 - 384, 16);
            et = mmc_get_bits(ssr, 402 - 384, 6);
            eo = mmc_get_bits(ssr, 400 - 384, 2);
            if (es && et) {
                card->card_reg.ssr.erase_timeout = (et * 1000) / es;
                card->card_reg.ssr.erase_offset = eo * 1000;
            }
            mmc_trace(MMC_TRACE_INFO, "au:0x%x, au size: %d", au,
                    card->card_reg.ssr.au_value);
        } else {
            mmc_printf(MMC_PRINTF_WARN, "SD Status: Invalid AU.\n");
        }
    }
err:
    free(ssr);
    return err;
}

static int sd_read_switch_func(struct mmc_card *card)
{
    int err = 0;
    unsigned char status[64];

    if (card->card_reg.scr.sda_vsn < SD_SCR_SPEC_VER_1)
        return 0;

    if (!(card->card_reg.csd.cmdclass & CCC_SWITCH)) {
        mmc_printf(MMC_PRINTF_WARN, "card lacks mandatory switch "
                "function, performance might suffer.\n");
        return 0;
    }
    memset(status, 0, 64);
    err = sd_switch_func(card, 0, 0, 1, status);
    if (err) {
        if (err != -ENOSYS && err != -EINVAL && err != -EFAULT)
            goto err;

        mmc_printf(MMC_PRINTF_WARN, "problem reading Bus Speed modes.\n");
        err = 0;

        goto err;
    }

    if (status[13] & SD_MODE_HIGH_SPEED)
        card->card_reg.sw_caps.hs_max_dtr = HIGH_SPEED_MAX_DTR;

    if (card->card_reg.scr.sda_spec3) {
        card->card_reg.sw_caps.sd3_bus_mode.data= status[13];

        err = sd_switch_func(card, 0, 2, 1, status);
        if (err) {
            if (err != -ENOSYS && err != -EFAULT && err != -EINVAL)
                goto err;

            mmc_printf(MMC_PRINTF_WARN, "problem reading "
                    "Driver Strength.\n");
            err = 0;
            goto err;
        }

        card->card_reg.sw_caps.sd3_drv_type = status[9];

        err = sd_switch_func(card, 0, 3, 1, status);
        if (err) {
            if (err != -EFAULT && err != -ENOSYS && err != -EINVAL)
                goto err;
            mmc_printf(MMC_PRINTF_WARN, "problem reading "
                    "Current Limit.\n");
            err = 0;
            goto err;
        }
        card->card_reg.sw_caps.sd3_curr_limit = status[7];
    } else {
        /* not use */
        if (status[13] & 0x02)
            card->card_reg.sw_caps.hs_max_dtr = 50000000;
    }
err:

    return err;
}

int sd_switch_func_hs(struct mmc_card *card)
{
    int err = 0;
    unsigned char *status;

    if (card->card_reg.sw_caps.hs_max_dtr == 0)
        return 0;
    if (!(card->host->caps.bits.cap_sd_highspeed))
        return 0;
    if (!(card->card_reg.csd.cmdclass & CCC_SWITCH))
        return 0;
    if (card->card_reg.scr.sda_vsn < SD_SCR_SPEC_VER_1)
        return 0;

    status = malloc(64);
    if (!status) {
        mmc_err("No memory for switch capabilities.\n");
        return -ENOMEM;
    }
    memset(status, 0, 64);
    err = sd_switch_func(card, 1, 0, 1, status);
    if (err)
        goto err;
    if ((status[16] & 0xF) != 1) {
        mmc_printf(MMC_PRINTF_ERR, "Problem switching card into high-speed mode!\n");
        err = 0;
    } else {
        err = 1;
    }
err:
    free(status);

    return err;
}

/* TODO:*/
static int sd_select_driver_type(struct mmc_card *card, unsigned char *status)
{
    int host_drv_type = 0, card_drv_type = 0;
    int err;
    return 0;
}

static void sd_update_bus_speed_mode(struct mmc_card *card)
{
    struct mmc_host *host = NULL;
    mmc_assert(card);
    host = card->host;
    if ( !(host->caps.bits.cap_UHS_SDR12 || \
                host->caps.bits.cap_UHS_SDR25 || \
                host->caps.bits.cap_UHS_SDR50 || \
                host->caps.bits.cap_UHS_SDR104 || \
                host->caps.bits.cap_UHS_DDR50)) {
        card->bus_speed_mode = SD_MODE_UHS_SDR12;
        return;
    }
    if ((card->card_reg.sw_caps.sd3_bus_mode.bits.uhs_sdr104) &&
            (host->caps.bits.cap_UHS_SDR104)) {
        card->bus_speed_mode = SD_MODE_UHS_SDR104;
    } else if ((card->card_reg.sw_caps.sd3_bus_mode.bits.uhs_ddr50) &&
            (host->caps.bits.cap_UHS_DDR50))  {
        card->bus_speed_mode = SD_MODE_UHS_DDR50;
    } else if ((host->caps.bits.cap_UHS_SDR104 | host->caps.bits.cap_UHS_SDR50) &&
            (card->card_reg.sw_caps.sd3_bus_mode.bits.uhs_sdr50)) {
        card->bus_speed_mode = SD_MODE_UHS_SDR50;
    } else if ((host->caps.bits.cap_UHS_SDR104 | host->caps.bits.cap_UHS_SDR50 |
                host->caps.bits.cap_UHS_SDR25) &&
            (card->card_reg.sw_caps.sd3_bus_mode.bits.hs_uhs_sdr25)) {
        card->bus_speed_mode = SD_MODE_UHS_SDR25;
    } else if ((host->caps.bits.cap_UHS_SDR104 | host->caps.bits.cap_UHS_SDR50 |
                host->caps.bits.cap_UHS_SDR25 |
                host->caps.bits.cap_UHS_SDR12) &&
            (card->card_reg.sw_caps.sd3_bus_mode.bits.uhs_sdr12)) {
        card->bus_speed_mode = SD_MODE_UHS_SDR12;
    }
}

static int sd_set_bus_speed_mode(struct mmc_card *card, unsigned char *status)
{
    int err = 0;
    enum mmc_bus_timing timing;

    switch (card->bus_speed_mode) {
        case SD_MODE_UHS_SDR104:
            timing = TIMING_UHS_SDR104;
            card->card_reg.sw_caps.uhs_max_dtr = UHS_SDR104_MAX_DTR;
            break;
        case SD_MODE_UHS_DDR50:
            timing = TIMING_UHS_DDR50;
            card->card_reg.sw_caps.uhs_max_dtr = UHS_DDR50_MAX_DTR;
            break;
        case SD_MODE_UHS_SDR50:
            timing = TIMING_UHS_SDR50;
            card->card_reg.sw_caps.uhs_max_dtr = UHS_SDR50_MAX_DTR;
            break;
        case SD_MODE_UHS_SDR25:
            timing = TIMING_UHS_SDR25;
            card->card_reg.sw_caps.uhs_max_dtr = UHS_SDR25_MAX_DTR;
            break;
        case SD_MODE_UHS_SDR12:
            timing = TIMING_UHS_SDR12;
            card->card_reg.sw_caps.uhs_max_dtr = UHS_SDR12_MAX_DTR;
            break;
        default:
            return 0;
    }

    err = sd_switch_func(card, 1, 0, card->bus_speed_mode, status);
    if (err) {
        return err;
    }
    if ((status[16] & 0xF) != card->bus_speed_mode)
        mmc_printf(MMC_PRINTF_WARN, "Warning: problem setting bus speed mode!,status = 0x%x\n",status[16]);
    else {
        card->iocfg.timing = timing;
        mmc_set_timing(card->host, card->iocfg.timing);
        card->iocfg.clock = card->card_reg.sw_caps.uhs_max_dtr;
        mmc_set_clock(card->host, card->iocfg.clock);
    }

    return 0;
}

static int sd_set_current_limit(struct mmc_card *card, unsigned char *status)
{
    int current_limit = 0;
    int err;
    struct mmc_host *host = NULL;
    host = card->host;

    if ((card->bus_speed_mode == SD_MODE_UHS_SDR104) ||
            (card->bus_speed_mode == SD_MODE_UHS_DDR50) ||
            (card->bus_speed_mode == SD_MODE_UHS_SDR50)) {
        if (host->caps.bits.cap_max_current_800) {
            if (card->card_reg.sw_caps.sd3_curr_limit == SD_MAX_CURRENT_800)
                current_limit = SD_SET_CURR_LIMIT_800;
            else if (card->card_reg.sw_caps.sd3_curr_limit == SD_MAX_CURRENT_600)
                current_limit = SD_SET_CURR_LIMIT_600;
            else if (card->card_reg.sw_caps.sd3_curr_limit == SD_MAX_CURRENT_400)
                current_limit = SD_SET_CURR_LIMIT_400;
            else if (card->card_reg.sw_caps.sd3_curr_limit == SD_MAX_CURRENT_200)
                current_limit = SD_SET_CURR_LIMIT_200;
        } else if (host->caps.bits.cap_max_current_600) {
            if (card->card_reg.sw_caps.sd3_curr_limit == SD_MAX_CURRENT_600)
                current_limit = SD_SET_CURR_LIMIT_600;
            else if (card->card_reg.sw_caps.sd3_curr_limit == SD_MAX_CURRENT_400)
                current_limit = SD_SET_CURR_LIMIT_400;
            else if (card->card_reg.sw_caps.sd3_curr_limit == SD_MAX_CURRENT_200)
                current_limit = SD_SET_CURR_LIMIT_200;
        } else if (host->caps.bits.cap_max_current_400) {
            if (card->card_reg.sw_caps.sd3_curr_limit == SD_MAX_CURRENT_400)
                current_limit = SD_SET_CURR_LIMIT_400;
            else if (card->card_reg.sw_caps.sd3_curr_limit == SD_MAX_CURRENT_200)
                current_limit = SD_SET_CURR_LIMIT_200;
        } else if (host->caps.bits.cap_max_current_200) {
            if (card->card_reg.sw_caps.sd3_curr_limit == SD_MAX_CURRENT_200)
                current_limit = SD_SET_CURR_LIMIT_200;
        }
    } else
        current_limit = SD_SET_CURR_LIMIT_200;

    err = sd_switch_func(card, 1, 3, current_limit, status);
    if (err)
        return err;

    if (((status[15] >> 4) & 0x0F) != current_limit)
        mmc_printf(MMC_PRINTF_WARN, "Problem setting current limit!\n");

    return 0;
}

static int sd_init_uhs_card(struct mmc_card *card)
{
    int err;
    unsigned char *status;

    if (!card->card_reg.scr.sda_spec3)
        return 0;

    if (!(card->card_reg.csd.cmdclass & CCC_SWITCH))
        return 0;

    status = malloc(64);
    if (!status) {
        mmc_err("No memory for switch capabilities.\n");
        return -ENOMEM;
    }

    if ((card->card_reg.scr.bus_widths & SD_SCR_BUS_WIDTH_4)\
            && (card->host->caps.bits.cap_4_bit)) {
        err = sd_app_set_bus_width(card, BUS_WIDTH_4);
        if (err)
            goto err;

        card->iocfg.bus_width = BUS_WIDTH_4;
        mmc_set_bus_width(card->host, BUS_WIDTH_4);
    }
    sd_update_bus_speed_mode(card);
    /* TODO: */
    //err = sd_select_driver_type(card, status);
    //if (err)
    //    goto err;

    err = sd_set_current_limit(card, status);
    if (err)
        goto err;

    err = sd_set_bus_speed_mode(card, status);
    if (err)
        goto err;

    if (SD_MODE_UHS_DDR50 == card->bus_speed_mode)
        err = mmc_execute_tuning(card->host, SD_SWITCH_FUNC);
    else if ((SD_MODE_UHS_SDR104 == card->bus_speed_mode)
            || (SD_MODE_UHS_SDR50 == card->bus_speed_mode))
        err = mmc_execute_tuning(card->host, SEND_TUNING_BLOCK);
err:
    free(status);

    return err;
}

static int sd_init_hs_card(struct mmc_card *card)
{
    int err = 0;
    struct mmc_host *host = NULL;
    mmc_assert(card);
    host = card->host;

    err = sd_switch_func_hs(card);
    if (err > 0)
        sd_go_highspeed(card);
    else if (err)
        goto out;

    mmc_set_clock(host, mmc_sd_get_max_clock(card));

    if ((host->caps.bits.cap_4_bit) &&
            (card->card_reg.scr.bus_widths & SD_SCR_BUS_WIDTH_4)) {
        err = sd_app_set_bus_width(card, BUS_WIDTH_4);
        if (err)
            goto out;
        card->iocfg.bus_width = BUS_WIDTH_4;
        mmc_set_bus_width(host, card->iocfg.bus_width);
    }
out:
    return err;
}

int sd_get_cid(struct mmc_card *card, union mmc_ocr ocr, uint32_t *cid, uint32_t *rocr)
{
    int err;
    struct mmc_host *host = NULL;
    mmc_assert(card);

    host = card->host;
    mmc_idle_card(card);
    err = sd_send_if_cond(card, ocr.ocr_data);
    if (!err)
        ocr.bits.HCS = 1;
    if (host->caps.bits.cap_XPC_330 || \
            host->caps.bits.cap_XPC_300 || \
            host->caps.bits.cap_XPC_180)
        ocr.bits.SD_XPC = 1;

    if (host->caps.bits.cap_UHS_SDR12 || \
            host->caps.bits.cap_UHS_SDR25 || \
            host->caps.bits.cap_UHS_SDR50 || \
            host->caps.bits.cap_UHS_SDR104 || \
            host->caps.bits.cap_UHS_DDR50)
        ocr.bits.S18 = 1;

try_again:
    err = sd_app_send_op_cond(card, ocr.ocr_data, rocr);
    if (err)
        return err;

    if (rocr && (*rocr & 0x41000000) == 0x41000000) {
        err = sd_signal_volt_switch(card);
        if (err) {
            ocr.bits.S18 = 0;
            goto try_again;
        }
        /* switch host and card to 1.8V */
        card->iocfg.vdd = VDD_1V8;
        err = mmc_voltage_switch(host, SIGNAL_VOLT_1V8);
        if (err) {
            ocr.bits.S18 = 0;
            goto try_again;
        }
    }
    err = mmc_all_send_cid(card, cid);

    return err;
}

int sd_get_csd(struct mmc_card *card)
{
    int err = 0;
    memset(card->card_reg.csd_raw, 0, 4*4);
    err = mmc_send_csd(card, (uint32_t *)card->card_reg.csd_raw);
    if (err)
        return err;

    err = sd_decode_csd(card);
    if (err)
        return err;

    return 0;
}

int sd_setup_card(struct mmc_card *card)
{
    int err;
    int ro = 0;

    err = sd_app_send_scr(card, (uint32_t *)card->card_reg.scr_raw);
    if (err)
        return err;

    err = sd_decode_scr(card);
    if (err)
        return err;

    err = mmc_read_ssr(card);
    if (err) {
        mmc_trace(3, "mmc_read_ssr err=%d", err);
        return err;
    }
    err = sd_read_switch_func(card);
    if (err) {
        mmc_trace(3, "err=%d", err);
        return err;
    }
    ro = mmc_get_readonly(card->host);
    if (ro)
        set_card_readonly(card);
    else
        mmc_printf(MMC_PRINTF_INFO, "Not read-only, anyhow!\n");
    return 0;
}

unsigned int mmc_sd_get_max_clock(struct mmc_card *card)
{
    unsigned int dtr = 0xFFFFFFFF;

    if (is_card_highspeed(card)) {
        if (card->card_reg.sw_caps.hs_max_dtr) {
            dtr = card->card_reg.sw_caps.hs_max_dtr;
        }
    } else if (card->card_reg.csd.max_dtr) {
        dtr = card->card_reg.csd.max_dtr;
    }

    return dtr;
}

void sd_go_highspeed(struct mmc_card *card)
{
    set_card_highspeed(card);
    card->iocfg.timing = TIMING_SD_HS;
    mmc_set_timing(card->host, card->iocfg.timing);
}

int sd_card_reset(struct mmc_card *card)
{
    int err = 0;
    if(!card) {
        mmc_err("card is null!");
        return -1;
    }
    err = mmc_assume_sd(card);
    if(err) {
        mmc_err("reset card fail!");
    } else {
        mmc_trace(5,"reset card success!");
    }
    return err;
}
int mmc_assume_sd(struct mmc_card *card)
{
    int err = 0;
    union mmc_ocr ocr;
    struct mmc_host *host = NULL;
    uint32_t cid[4];

    mmc_assert(card);
    host = card->host;
    err = sd_app_send_op_cond(card, 0, &(ocr.ocr_data));
    if (err) {
        return err;
    }

    if (ocr.ocr_data & 0x7F) {
        mmc_printf(MMC_PRINTF_WARN, "Card claim to support voltages "
                "below the defined range. be ignored.\n");
        ocr.ocr_data &= (~0x7F);
    }

    if ((ocr.bits.vdd_1v65_1v95) &&
            !(host->ocr_default.bits.vdd_1v65_1v95)) {
        mmc_printf(MMC_PRINTF_WARN, "SD card claim to support the "
                "'low voltage range'. be ignored.\n");
        ocr.bits.vdd_1v65_1v95 = 0;
    }

    card->card_reg.ocr.ocr_data = mmc_select_voltage(host, ocr.ocr_data);
    if (!card->card_reg.ocr.ocr_data) {
        err = -EINVAL;
        goto err;
    }
    err = sd_get_cid(card, card->card_reg.ocr, cid, &ocr.ocr_data);
    if (err)
        goto err;
    /* now card is identified */
    set_card_type(card, CARD_TYPE_SD);
    set_card_removeable(card);
    err = sd_send_relative_addr(card, &(card->card_reg.rca));
    if (err)
        goto err;

    err = sd_get_csd(card);
    if (err)
        goto err;
    sd_decode_cid(card);

    err = mmc_select_card(card);
    if (err)
        goto err;

    err = sd_setup_card(card);
    if(err)
        goto err;

    if (ocr.bits.S18) {
        /* uhs card set */
        err = sd_init_uhs_card(card);
        if (err)
            goto err;
        set_card_uhs(card);
    } else {
        /* highspeed card set */
        err = sd_init_hs_card(card);
        if (err)
            goto err;
    }
    err = mmc_add_card(card);
    if (err)
        goto err;
    return 0;

err:
    mmc_reset_card(card);
    mmc_printf(MMC_PRINTF_ERR, "error %d initializing SD card\n",
            err);
    return err;
}

/* end of file sd.c*/
