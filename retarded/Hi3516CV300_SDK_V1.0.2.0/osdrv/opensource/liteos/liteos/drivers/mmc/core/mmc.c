/*mmc.c begin*/
#include "mmc_core.h"

extern const uint32_t tran_exp[];
extern const uint8_t tran_mant[];
extern const uint32_t tacc_exp[];
extern const uint32_t tacc_mant[];


static int mmc_decode_cid(struct mmc_card *card)
{
    int i;
    struct mmc_cid *cid = &(card->card_reg.cid);
    uint32_t *cid_raw = card->card_reg.cid_raw;
    uint8_t spec_vers = card->card_reg.csd.spec_vers;

    if ((0 == spec_vers) || (1 == spec_vers)) {
        cid->mid = mmc_get_bits(cid_raw, 104, 24);
        for (i = 0; i < 7; i++)
            cid->pnm[i] = mmc_get_bits(cid_raw, 96 - i * 8, 8);
        cid->pnm[7] = 0;
        cid->hwrev = mmc_get_bits(cid_raw, 44, 4);
        cid->fwrev = mmc_get_bits(cid_raw, 40, 4);
        cid->serial = mmc_get_bits(cid_raw, 16, 24);
        cid->month = mmc_get_bits(cid_raw, 12, 4);
        cid->year = mmc_get_bits(cid_raw, 8, 4) + 1997;
    } else if ((2 <= spec_vers) && (4 >= spec_vers)) {
        cid->mid = mmc_get_bits(cid_raw, 120, 8);
        cid->oid = mmc_get_bits(cid_raw, 104, 16);
        for (i = 0; i < 6; i++)
            cid->pnm[i] = mmc_get_bits(cid_raw, 96 - i * 8, 8);
        cid->pnm[6] = 0;
        cid->serial = mmc_get_bits(cid_raw, 16, 32);
        cid->month = mmc_get_bits(cid_raw, 12, 4);
        cid->year = mmc_get_bits(cid_raw, 8, 4);
    } else {
        mmc_printf(MMC_PRINTF_ERR, "card with unknown mcca version %d\n",
                card->card_reg.csd.spec_vers);
        return -EINVAL;
    }
    return 0;
}

static void mmc_set_erase_size(struct mmc_card *card)
{
    if (card->card_reg.ext_csd.erase_group_def & 1)
        card->erase_size = card->card_reg.ext_csd.hc_erase_size;
    else
        card->erase_size = card->card_reg.csd.erase_size;
    /* todo */
    /*mmc_init_erase(card);*/
}

static int mmc_decode_csd(struct mmc_card *card)
{
    struct mmc_csd *csd = &card->card_reg.csd;
    uint32_t e, m, a, b;
    uint32_t *csd_raw = card->card_reg.csd_raw;

    csd->structure = mmc_get_bits(csd_raw, 126, 2);
    if (0 == csd->structure) {
        mmc_printf(MMC_PRINTF_ERR, "unknown CSD structure version %d\n",
                csd->structure);
        return -EINVAL;
    }

    csd->spec_vers = mmc_get_bits(csd_raw, 122, 4);
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
    csd->capacity = (1 + m) << (e + 2); /*block number */

    csd->read_blkbits = mmc_get_bits(csd_raw, 80, 4);
    csd->read_partial = mmc_get_bits(csd_raw, 79, 1);
    csd->write_misalign = mmc_get_bits(csd_raw, 78, 1);
    csd->read_misalign = mmc_get_bits(csd_raw, 77, 1);
    csd->r2w_factor = mmc_get_bits(csd_raw, 26, 3);
    csd->write_blkbits = mmc_get_bits(csd_raw, 22, 4);
    csd->write_partial = mmc_get_bits(csd_raw, 21, 1);

    if (csd->write_blkbits >= 9) {
        a = mmc_get_bits(csd_raw, 42, 5);
        b = mmc_get_bits(csd_raw, 37, 5);
        csd->erase_size = (a + 1) * (b + 1);
        csd->erase_size <<= csd->write_blkbits - 9;
    }

    return 0;
}

static int mmc_get_ext_csd(struct mmc_card *card, uint8_t **new_ext_csd)
{
    int err = 0;
    uint8_t *ext_csd;

    *new_ext_csd = NULL;

    if (card->card_reg.csd.spec_vers < CSD_SPEC_VER_4)
        return 0;

    ext_csd = malloc(512);
    if (!ext_csd) {
        mmc_printf(MMC_PRINTF_ERR, "No memory for ext_csd.\n");
        return -ENOMEM;
    }
    err = mmc_send_ext_csd(card, ext_csd);
    if (err) {
        free(ext_csd);
        *new_ext_csd = NULL;
    } else
        *new_ext_csd = ext_csd;

    return err;
}

/*
 * Decode extended CSD.
 */
static int mmc_decode_ext_csd(struct mmc_card *card, uint8_t *ext_csd)
{
    int err = 0;
    struct mmc_ext_csd *ext_csd_reg;
    if (!ext_csd)
        return 0;

    ext_csd_reg = &(card->card_reg.ext_csd);

    ext_csd_reg->raw_ext_csd_structure = ext_csd[E_CSD_STRUCTURE];
    if (card->card_reg.csd.structure == 3) {
        if (ext_csd_reg->raw_ext_csd_structure > 2) {
            mmc_printf(MMC_PRINTF_ERR, "unrecognised EXT_CSD structure "
                    "version %d\n",
                    ext_csd_reg->raw_ext_csd_structure);
            err = -EINVAL;
            goto out;
        }
    }
    ext_csd_reg->rev = ext_csd[E_CSD_REV];

    ext_csd_reg->raw_sectors[0] = ext_csd[E_CSD_SEC_CNT + 0];
    ext_csd_reg->raw_sectors[1] = ext_csd[E_CSD_SEC_CNT + 1];
    ext_csd_reg->raw_sectors[2] = ext_csd[E_CSD_SEC_CNT + 2];
    ext_csd_reg->raw_sectors[3] = ext_csd[E_CSD_SEC_CNT + 3];
    if (ext_csd_reg->rev >= 2) {
        ext_csd_reg->sectors =
            ext_csd[E_CSD_SEC_CNT + 0] << 0 |
            ext_csd[E_CSD_SEC_CNT + 1] << 8 |
            ext_csd[E_CSD_SEC_CNT + 2] << 16 |
            ext_csd[E_CSD_SEC_CNT + 3] << 24;

        /* sector addressed */
        if (ext_csd_reg->sectors > (2u * 1024 * 1024 * 1024) / 512)
            set_card_blockaddr(card);
    }
    ext_csd_reg->raw_card_type = ext_csd[E_CSD_CARD_TYPE];

    switch (ext_csd[E_CSD_CARD_TYPE] & E_CSD_CARD_TYPE_MASK) {
        case E_CSD_CARD_TYPE_SDR_ALL:
        case E_CSD_CARD_TYPE_SDR_ALL_DDR_1_8V:
        case E_CSD_CARD_TYPE_SDR_ALL_DDR_1_2V:
        case E_CSD_CARD_TYPE_SDR_ALL_DDR_52:
            ext_csd_reg->hs_max_dtr = 200000000;
            ext_csd_reg->card_type = E_CSD_CARD_TYPE_SDR_200;
            break;
        case E_CSD_CARD_TYPE_SDR_1_2V_ALL:
        case E_CSD_CARD_TYPE_SDR_1_2V_DDR_1_8V:
        case E_CSD_CARD_TYPE_SDR_1_2V_DDR_1_2V:
        case E_CSD_CARD_TYPE_SDR_1_2V_DDR_52:
            ext_csd_reg->hs_max_dtr = 200000000;
            ext_csd_reg->card_type = E_CSD_CARD_TYPE_SDR_1_2V;
            break;
        case E_CSD_CARD_TYPE_SDR_1_8V_ALL:
        case E_CSD_CARD_TYPE_SDR_1_8V_DDR_1_8V:
        case E_CSD_CARD_TYPE_SDR_1_8V_DDR_1_2V:
        case E_CSD_CARD_TYPE_SDR_1_8V_DDR_52:
            ext_csd_reg->hs_max_dtr = 200000000;
            ext_csd_reg->card_type = E_CSD_CARD_TYPE_SDR_1_8V;
            break;
        case E_CSD_CARD_TYPE_DDR_52 | E_CSD_CARD_TYPE_52 | E_CSD_CARD_TYPE_26:
            ext_csd_reg->hs_max_dtr = 52000000;
            ext_csd_reg->card_type = E_CSD_CARD_TYPE_DDR_52;
            break;
        case E_CSD_CARD_TYPE_DDR_1_2V | E_CSD_CARD_TYPE_52 | E_CSD_CARD_TYPE_26:
            ext_csd_reg->hs_max_dtr = 52000000;
            ext_csd_reg->card_type = E_CSD_CARD_TYPE_DDR_1_2V;
            break;
        case E_CSD_CARD_TYPE_DDR_1_8V | E_CSD_CARD_TYPE_52 | E_CSD_CARD_TYPE_26:
            ext_csd_reg->hs_max_dtr = 52000000;
            ext_csd_reg->card_type = E_CSD_CARD_TYPE_DDR_1_8V;
            break;
        case E_CSD_CARD_TYPE_52 | E_CSD_CARD_TYPE_26:
            ext_csd_reg->hs_max_dtr = 52000000;
            break;
        case E_CSD_CARD_TYPE_26:
            ext_csd_reg->hs_max_dtr = 26000000;
            break;
        default:
            mmc_printf(MMC_PRINTF_ERR, "card is mmc v4 without "
                    "support high-speed");
        break;
    }

    ext_csd_reg->raw_s_a_timeout = ext_csd[E_CSD_S_A_TIMEOUT];
    ext_csd_reg->raw_erase_timeout_mult =
        ext_csd[E_CSD_ERASE_TIMEOUT_MULT];
    ext_csd_reg->raw_hc_erase_grp_size =
        ext_csd[E_CSD_HC_ERASE_GRP_SIZE];
    if (ext_csd_reg->rev >= 3) {
        uint8_t sa_shift = ext_csd[E_CSD_S_A_TIMEOUT];
        ext_csd_reg->part_config = ext_csd[E_CSD_PART_CONFIG];

        ext_csd_reg->part_time_ms = 10 * ext_csd[E_CSD_PART_SWITCH_TIME];

        if (sa_shift > 0 && sa_shift <= 0x17)
            ext_csd_reg->sa_timeout_ns =
                1 << ext_csd[E_CSD_S_A_TIMEOUT];
        ext_csd_reg->erase_group_def =
            ext_csd[E_CSD_ERASE_GROUP_DEF];
        ext_csd_reg->hc_erase_timeout = 300 *
            ext_csd[E_CSD_ERASE_TIMEOUT_MULT];
        ext_csd_reg->hc_erase_size =
            ext_csd[E_CSD_HC_ERASE_GRP_SIZE] << 10;

        ext_csd_reg->rel_sectors = ext_csd[E_CSD_REL_WR_SEC_C];

        ext_csd_reg->boot_size = ext_csd[E_CSD_BOOT_MULT] << 17;
    }

    ext_csd_reg->raw_hc_erase_gap_size =
        ext_csd[E_CSD_PARTITION_ATTRIBUTE];
    ext_csd_reg->raw_sec_trim_mult =
        ext_csd[E_CSD_SEC_TRIM_MULT];
    ext_csd_reg->raw_sec_erase_mult =
        ext_csd[E_CSD_SEC_ERASE_MULT];
    ext_csd_reg->raw_sec_feature_support =
        ext_csd[E_CSD_SEC_FEATURE_SUPPORT];
    ext_csd_reg->raw_trim_mult =
        ext_csd[E_CSD_TRIM_MULT];
    if (ext_csd_reg->rev >= 4) {
        if ((ext_csd[E_CSD_PARTITION_SUPPORT] & 0x2) &&
                (ext_csd[E_CSD_PARTITION_ATTRIBUTE] & 0x1)) {
            uint8_t hc_erase_grp_sz =
                ext_csd[E_CSD_HC_ERASE_GRP_SIZE];
            uint8_t hc_wp_grp_sz =
                ext_csd[E_CSD_HC_WP_GRP_SIZE];

            ext_csd_reg->enhanced_area_en = 1;
            ext_csd_reg->enhanced_area_offset =
                (uint64_t)(ext_csd[139] << 24) + (ext_csd[138] << 16) +
                (ext_csd[137] << 8) + ext_csd[136];
            if (is_card_blkaddr(card))
                ext_csd_reg->enhanced_area_offset <<= 9;
            ext_csd_reg->enhanced_area_size =
                (ext_csd[142] << 16) + (ext_csd[141] << 8) +
                ext_csd[140];
            ext_csd_reg->enhanced_area_size *=
                (uint32_t)(hc_erase_grp_sz * hc_wp_grp_sz);
            ext_csd_reg->enhanced_area_size <<= 9;
        } else {
            ext_csd_reg->enhanced_area_offset = -EINVAL;
            ext_csd_reg->enhanced_area_size = -EINVAL;
        }
        ext_csd_reg->sec_trim_mult =
            ext_csd[E_CSD_SEC_TRIM_MULT];
        ext_csd_reg->sec_erase_mult =
            ext_csd[E_CSD_SEC_ERASE_MULT];
        ext_csd_reg->sec_feature_support =
            ext_csd[E_CSD_SEC_FEATURE_SUPPORT];
        ext_csd_reg->trim_timeout = 300 *
            ext_csd[E_CSD_TRIM_MULT];
    }

    if (ext_csd_reg->rev >= 5) {
        /* check whether the eMMC card supports HPI */
        if (ext_csd[E_CSD_HPI_FEATURES] & 0x1) {
            ext_csd_reg->hpi = 1;
            if (ext_csd[E_CSD_HPI_FEATURES] & 0x2)
                ext_csd_reg->hpi_cmd = STOP_TRANSMISSION;
            else
                ext_csd_reg->hpi_cmd = SEND_STATUS;
            /*
             ** Indicate the maximum timeout to close
             ** a command interrupted by HPI
             **/
            ext_csd_reg->out_of_int_time =
                ext_csd[E_CSD_OUT_OF_INTERRUPT_TIME] * 10;
        }
        ext_csd_reg->rel_param = ext_csd[E_CSD_WR_REL_PARAM];
    }
    if (ext_csd[E_CSD_ERASED_MEM_CONT])
        card->erased_byte = 0xFF;
    else
        card->erased_byte = 0x0;
out:
    return err;
}

static void mmc_free_ext_csd(uint8_t *ext_csd)
{
    free(ext_csd);
    ext_csd = NULL;
}

static int mmc_compare_ext_csds(struct mmc_card *card, enum mmc_bus_width bus_width)
{
    uint8_t *bw_ext_csd;
    int err = 0;
    struct mmc_ext_csd *ext_csd_reg;

    ext_csd_reg = &(card->card_reg.ext_csd);

    if (bus_width == BUS_WIDTH_1)
        return 0;

    err = mmc_get_ext_csd(card, &bw_ext_csd);

    if (err || bw_ext_csd == NULL) {
            err = -EINVAL;
        goto out;
    }
#if 0
    err = !((ext_csd_reg->raw_partition_support ==
            bw_ext_csd[E_CSD_PARTITION_SUPPORT]) &&
        (ext_csd_reg->raw_erased_mem_count ==
            bw_ext_csd[E_CSD_ERASED_MEM_CONT]) &&
        (ext_csd_reg->rev ==
            bw_ext_csd[E_CSD_REV]) &&
        (ext_csd_reg->raw_ext_csd_structure ==
            bw_ext_csd[E_CSD_STRUCTURE]) &&
        (ext_csd_reg->raw_card_type ==
            bw_ext_csd[E_CSD_CARD_TYPE]) &&
        (ext_csd_reg->raw_s_a_timeout ==
            bw_ext_csd[E_CSD_S_A_TIMEOUT]) &&
        (ext_csd_reg->raw_hc_erase_gap_size ==
            bw_ext_csd[E_CSD_HC_WP_GRP_SIZE]) &&
        (ext_csd_reg->raw_erase_timeout_mult ==
            bw_ext_csd[E_CSD_ERASE_TIMEOUT_MULT]) &&
        (ext_csd_reg->raw_hc_erase_grp_size ==
            bw_ext_csd[E_CSD_HC_ERASE_GRP_SIZE]) &&
        (ext_csd_reg->raw_sec_trim_mult ==
            bw_ext_csd[E_CSD_SEC_TRIM_MULT]) &&
        (ext_csd_reg->raw_sec_erase_mult ==
            bw_ext_csd[E_CSD_SEC_ERASE_MULT]) &&
        (ext_csd_reg->raw_sec_feature_support ==
            bw_ext_csd[E_CSD_SEC_FEATURE_SUPPORT]) &&
        (ext_csd_reg->raw_trim_mult ==
            bw_ext_csd[E_CSD_TRIM_MULT]) &&
        (ext_csd_reg->raw_sectors[0] ==
            bw_ext_csd[E_CSD_SEC_CNT + 0]) &&
        (ext_csd_reg->raw_sectors[1] ==
            bw_ext_csd[E_CSD_SEC_CNT + 1]) &&
        (ext_csd_reg->raw_sectors[2] ==
            bw_ext_csd[E_CSD_SEC_CNT + 2]) &&
        (ext_csd_reg->raw_sectors[3] ==
            bw_ext_csd[E_CSD_SEC_CNT + 3]));
#endif
out:
    mmc_free_ext_csd(bw_ext_csd);
    return err;
}

static int mmc_select_hs200(struct mmc_card *card)
{
    int idx, err = 0;
    struct mmc_host *host;
    unsigned ext_csd_bits[] = {
        E_CSD_BUS_WIDTH_4,
        E_CSD_BUS_WIDTH_8,
    };
    enum mmc_bus_width bus_widths[] = {
        BUS_WIDTH_4,
        BUS_WIDTH_8,
    };

    host = card->host;

    if (host->caps2.bits.caps2_HS200_1v2_SDR &&
            card->card_reg.ext_csd.card_type & E_CSD_CARD_TYPE_SDR_1_2V) {
        mmc_voltage_switch(host, SIGNAL_VOLT_1V2);
    }
    if (host->caps2.bits.caps2_HS200_1v8_SDR &&
            card->card_reg.ext_csd.card_type & E_CSD_CARD_TYPE_SDR_1_8V) {
        mmc_voltage_switch(host, SIGNAL_VOLT_1V8);
    }
    idx = (host->caps.bits.cap_8_bit);
    for (; idx >= 0; idx--) {
        err = mmc_switch(card, E_CSD_CMD_SET_NORMAL,
                E_CSD_BUS_WIDTH, ext_csd_bits[idx],
                0);
        if (err)
            continue;

        card->iocfg.bus_width = bus_widths[idx];
        mmc_set_bus_width(card->host, bus_widths[idx]);

        if (!(host->caps.bits.cap_bus_width_test))
            err = mmc_compare_ext_csds(card, bus_widths[idx]);
        if (!err)
            break;
    }
    if (!err) {
        err = mmc_switch(card, E_CSD_CMD_SET_NORMAL,
                E_CSD_HS_TIMING, 2, 0);

    }
err:
    return err;
}

static int mmc_select_powerclass(struct mmc_card *card,
        uint32_t bus_width, uint8_t *ext_csd)
{
    int err = 0;
    uint32_t pwrclass_val;
    uint32_t index = 0;
    struct mmc_host *host;

    mmc_assert(card);

    host = card->host;

    if (ext_csd == NULL)
        return 0;

    if (card->card_reg.csd.spec_vers < CSD_SPEC_VER_4)
        return 0;

    if (bus_width == E_CSD_BUS_WIDTH_1)
        return 0;
    switch (1 << host->dvdd) {
        case MMC_1V65_1V95:
            if (card->iocfg.clock <= 26000000)
                index = E_CSD_PWR_CL_26_195;
            else if (card->iocfg.clock <= 52000000)
                index = (bus_width <= E_CSD_BUS_WIDTH_8) ?
                    E_CSD_PWR_CL_52_195 :
                    E_CSD_PWR_CL_DDR_52_195;
            else if (card->iocfg.clock <= 200000000)
                index = E_CSD_PWR_CL_200_195;
            break;
        case MMC_2V7_2V8:
        case MMC_2V8_2V9:
        case MMC_2V9_3V0:
        case MMC_3V0_3V1:
        case MMC_3V1_3V2:
        case MMC_3V2_3V3:
        case MMC_3V3_3V4:
        case MMC_3V4_3V5:
        case MMC_3V5_3V6:
            if (card->iocfg.clock <= 26000000)
                index = E_CSD_PWR_CL_26_360;
            else if (card->iocfg.clock <= 52000000)
                index = (bus_width <= E_CSD_BUS_WIDTH_8) ?
                    E_CSD_PWR_CL_52_360 :
                    E_CSD_PWR_CL_DDR_52_360;
            else if (card->iocfg.clock <= 200000000)
                index = E_CSD_PWR_CL_200_360;
            break;
        default:
            mmc_printf(MMC_PRINTF_WARN, "Voltage range not supported "
                    "for power class.\n");
            return -EINVAL;
    }

    pwrclass_val = ext_csd[index];

    if (bus_width & (E_CSD_BUS_WIDTH_8 | E_CSD_DDR_BUS_WIDTH_8))
        pwrclass_val = (pwrclass_val & E_CSD_PWR_CL_8BIT_MASK) >>
            E_CSD_PWR_CL_8BIT_SHIFT;
    else
        pwrclass_val = (pwrclass_val & E_CSD_PWR_CL_4BIT_MASK) >>
            E_CSD_PWR_CL_4BIT_SHIFT;

    if (pwrclass_val > 0) {
        err = mmc_switch(card, E_CSD_CMD_SET_NORMAL,
                E_CSD_POWER_CLASS,
                pwrclass_val,
                card->card_reg.ext_csd.generic_cmd6_time_ms);
    }

    return err;
}

static void mmc_check_removeable(struct mmc_card *card)
{
    int cbx = 0;
    uint32_t *cid_raw = card->card_reg.cid_raw;

    cbx = mmc_get_bits(cid_raw, 112, 2);
    if ((cbx == 0x1 || cbx == 0x2))
       clr_card_removeable(card);
    else
       set_card_removeable(card);
}

static int mmc_setup_card(struct mmc_card *card, union mmc_ocr ocr)
{
    struct mmc_host *host;
    int err, ddr = 0;
    uint32_t cid[4];
    uint32_t max_dtr;
    union mmc_ocr rocr;
    uint8_t *ext_csd = NULL;
    mmc_assert(card);
    host = card->host;

    host->volt_default = SIGNAL_VOLT_3V3;
    mmc_voltage_switch(host, host->volt_default);

    mmc_idle_card(card);
    ocr.bits.HCS = 1;
    err = mmc_send_op_cond(card, ocr.ocr_data, &rocr.ocr_data);
    if (err)
        goto err;

    err = mmc_all_send_cid(card, cid);
    if (err)
        goto err;

    /* card is identified */
    set_card_type(card, CARD_TYPE_MMC);
    card->card_reg.rca = 1;
    memcpy(card->card_reg.cid_raw, cid, sizeof(card->card_reg.cid_raw));
    mmc_check_removeable(card);

    err = mmc_set_relative_addr(card);
    if (err)
        goto err;
    mmc_set_bus_mode(host, MMC_BUSMODE_PUSHPULL);

    /*
     * Get CSD.
     */
    err = mmc_send_csd(card, (uint32_t *)card->card_reg.csd_raw);
    if (err)
        goto err;
    err = mmc_decode_csd(card);
    if (err)
        goto err;
    /* we decode cid after csd decoded */
    err = mmc_decode_cid(card);
    if (err)
        goto err;

    /* TODO: DSR */

    /* Select card */
    err = mmc_select_card(card);
    if (err)
        goto err;

    /* Get and decode extended CSD */
    err = mmc_get_ext_csd(card, &ext_csd);
    if (err)
        goto err;
    err = mmc_decode_ext_csd(card, ext_csd);
    if (err)
        goto err;

    if (!(is_card_blkaddr(card)) && (rocr.bits.HCS))
        set_card_blockaddr(card);

    mmc_set_erase_size(card);

    if (card->card_reg.ext_csd.enhanced_area_en) {
        err = mmc_switch(card, E_CSD_CMD_SET_NORMAL,
                E_CSD_ERASE_GROUP_DEF, 1, 0);

        if (err && err != -EBADMSG)
            goto err;

        if (err) {
            err = 0;
            card->card_reg.ext_csd.enhanced_area_offset = -EINVAL;
            card->card_reg.ext_csd.enhanced_area_size = -EINVAL;
        } else {
            card->card_reg.ext_csd.erase_group_def = 1;
            mmc_set_erase_size(card);
        }
    }

    if (card->card_reg.ext_csd.part_config & E_CSD_PART_CONFIG_ACC_MASK) {
        card->card_reg.ext_csd.part_config &= ~E_CSD_PART_CONFIG_ACC_MASK;
        err = mmc_switch(card, E_CSD_CMD_SET_NORMAL, E_CSD_PART_CONFIG,
                card->card_reg.ext_csd.part_config,
                card->card_reg.ext_csd.part_time_ms);
        if (err && err != -EBADMSG)
            goto err;
    }

    /*
     * Activate high speed (if supported)
     */
    if (card->card_reg.ext_csd.hs_max_dtr != 0) {
        err = 0;
        if (card->card_reg.ext_csd.hs_max_dtr > 52000000 &&
                (host->caps2.bits.caps2_HS200_1v8_SDR |
                host->caps2.bits.caps2_HS200_1v2_SDR))
            err = mmc_select_hs200(card);
        else if (host->caps.bits.cap_mmc_highspeed)
            err = mmc_switch(card, E_CSD_CMD_SET_NORMAL,
                    E_CSD_HS_TIMING, 1,
                    card->card_reg.ext_csd.generic_cmd6_time_ms);

        if (err && err != -EBADMSG)
            goto err;

        if (err) {
            mmc_printf(MMC_PRINTF_WARN, "switch to highspeed failed\n");
            err = 0;
        } else {
            if (card->card_reg.ext_csd.hs_max_dtr > 52000000 &&
                    ((host->caps2.bits.caps2_HS200_1v8_SDR) |
                     (host->caps2.bits.caps2_HS200_1v2_SDR))){
                set_card_hs200(card);
                card->iocfg.timing = TIMING_MMC_HS200;
                mmc_set_timing(card->host, card->iocfg.timing);
            } else {
                set_card_highspeed(card);
                card->iocfg.timing = TIMING_MMC_HS;
                mmc_set_timing(card->host, card->iocfg.timing);
            }
        }
    }

    /*
     * Compute bus speed.
     */
    max_dtr = 0xFFFFFFFF;

    if (is_card_highspeed(card) ||  is_card_hs200(card)) {
        if (card->card_reg.ext_csd.hs_max_dtr)
            max_dtr = card->card_reg.ext_csd.hs_max_dtr;
    } else if (card->card_reg.csd.max_dtr) {
        max_dtr = card->card_reg.csd.max_dtr;
    }

    card->iocfg.clock = max_dtr;
    mmc_set_clock(host, card->iocfg.clock);

    /*
     * Indicate DDR mode (if supported).
     */
    if (is_card_highspeed(card)) {
        if ((card->card_reg.ext_csd.card_type & E_CSD_CARD_TYPE_DDR_1_8V)
                && ((host->caps.bits.cap_1v8_ddr) &
                    (host->caps.bits.cap_UHS_DDR50)))
            ddr = MMC_1_8V_DDR_MODE;
        else if ((card->card_reg.ext_csd.card_type & E_CSD_CARD_TYPE_DDR_1_2V)
                && ((host->caps.bits.cap_1v2_ddr) &
                        (host->caps.bits.cap_1v2_ddr)))
            ddr = MMC_1_2V_DDR_MODE;
    }

    if (is_card_hs200(card)) {
        uint32_t ext_csd_bits;
        enum mmc_bus_width bus_width = card->iocfg.bus_width;
        if ((host->caps2.bits.caps2_HS200_1v8_SDR | host->caps2.bits.caps2_HS200_1v2_SDR)) {
            mmc_execute_tuning(card->host, SEND_TUNING_BLOCK_HS200);
        }
        if (err) {
            mmc_printf(MMC_PRINTF_WARN, "tuning execution failed\n");
            goto err;
        }

        ext_csd_bits = (bus_width == BUS_WIDTH_8) ?
            E_CSD_BUS_WIDTH_8 : E_CSD_BUS_WIDTH_4;
        err = mmc_select_powerclass(card, ext_csd_bits, ext_csd);
        if (err)
            mmc_printf(MMC_PRINTF_WARN, "power class selection to bus width %d"
                    " failed\n",1 << bus_width);
    }
    if (!is_card_hs200(card) && (card->card_reg.csd.spec_vers >= CSD_SPEC_VER_4) &&
            (host->caps.bits.cap_4_bit | host->caps.bits.cap_4_bit)) {
        unsigned ext_csd_bits[][2] = {
            { E_CSD_BUS_WIDTH_8, E_CSD_DDR_BUS_WIDTH_8 },
            { E_CSD_BUS_WIDTH_4, E_CSD_DDR_BUS_WIDTH_4 },
            { E_CSD_BUS_WIDTH_1, E_CSD_BUS_WIDTH_1 },
        };
        enum mmc_bus_width bus_widths[] = {
            BUS_WIDTH_8,
            BUS_WIDTH_4,
            BUS_WIDTH_1
        };
        unsigned idx;
        enum mmc_bus_width bus_width = card->iocfg.bus_width;
        if (host->caps.bits.cap_8_bit)
            idx = 0;
        else
            idx = 1;
        for (; idx < ARRAY_SIZE(bus_widths); idx++) {
            bus_width = bus_widths[idx];
            if (bus_width == BUS_WIDTH_1)
                ddr = 0; /* no DDR for 1-bit width */
            err = mmc_switch(card, E_CSD_CMD_SET_NORMAL,
                    E_CSD_BUS_WIDTH,
                    ext_csd_bits[idx][0],
                    0);
            if (!err) {
                card->iocfg.bus_width = bus_width;
                mmc_set_bus_width(card->host, card->iocfg.bus_width);

                err = mmc_compare_ext_csds(card, bus_width);
                if (!err)
                    break;
            }
        }

        if (!err && ddr) {
            err = mmc_switch(card, E_CSD_CMD_SET_NORMAL,
                    E_CSD_BUS_WIDTH,
                    ext_csd_bits[idx][1],
                    0);
        }
        if (err) {
            mmc_printf(MMC_PRINTF_WARN, "switch to bus width %d ddr %d "
                    "failed\n",
                    1 << bus_width, ddr);
            goto err;
        } else if (ddr) {
            if (ddr == E_CSD_CARD_TYPE_DDR_1_2V) {
                err = mmc_voltage_switch(host, SIGNAL_VOLT_1V2);
                if (err)
                    goto err;
            }
            set_card_ddr_mode(card);
            card->iocfg.timing = TIMING_UHS_DDR50;
            mmc_set_timing(card->host, card->iocfg.timing);
            card->iocfg.bus_width = bus_width;
            mmc_set_bus_width(card->host, card->iocfg.bus_width);
        }
    }
    if (card->card_reg.ext_csd.hpi) {
        err = mmc_switch(card, E_CSD_CMD_SET_NORMAL,
                E_CSD_HPI_MGMT, 1,
                card->card_reg.ext_csd.generic_cmd6_time_ms);
        if (err && err != -EBADMSG)
            goto err;
        if (err) {
            mmc_printf(MMC_PRINTF_WARN, "Enabling HPI failed\n");
        } else
            card->card_reg.ext_csd.hpi_en = 1;
    }

    mmc_free_ext_csd(ext_csd);
    return 0;

err:
    mmc_free_ext_csd(ext_csd);

    return err;
}


/* detect as a mmc or emmc card */
int mmc_assume_mmc(struct mmc_card *card)
{
    int err;
    struct mmc_host *host;
    union mmc_ocr ocr;
    mmc_assert(card);
    host = card->host;

    err = mmc_send_op_cond(card, 0, &ocr.ocr_data);
    if (err)
        return err;

    if (ocr.ocr_data & 0x7F) {
        mmc_printf(MMC_PRINTF_WARN, "card claim to support voltages "
                "below the defined range. ignored.\n");
        ocr.ocr_data &= ~0x7F;
    }

    card->card_reg.ocr.ocr_data = mmc_select_voltage(host, ocr.ocr_data);

    if (!card->card_reg.ocr.ocr_data) {
        err = -EINVAL;
        goto err;
    }
    /* detect and setup the card */
    err = mmc_setup_card(card, card->card_reg.ocr);
    if (err) {
        mmc_printf(MMC_PRINTF_ERR, "mmc setup card fail!\n");
        goto err;
    }

    err = mmc_add_card(card);
    if (err) {
        mmc_printf(MMC_PRINTF_ERR, "mmc add card fail!\n");
        goto err;
    }
    return 0;

err:
    //mmc_reset_card(card);
    mmc_printf(MMC_PRINTF_ERR, "error %d initializing MMC card\n", err);

    return err;
}

int get_card_status(struct mmc_card *card, unsigned int *status)
{
    struct mmc_cmd cmd = {0};
    int err;

    cmd.cmd_code = SEND_STATUS;
    cmd.arg = card->card_reg.rca << 16;
    cmd.resp_type =  MMC_RESP_R1 | MMC_CMD_AC;

    err = mmc_wait_for_cmd(card->host, &cmd, 0);
    if (err == 0) {
        *status = cmd.resp[0];
        mmc_trace(5,"status = 0x%x",status);
    }
    return err;
}
/* end of file mmc.c */
