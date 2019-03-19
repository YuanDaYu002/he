/* card.c begin */
#include "mmc_core.h"

struct mmc_card *mmc_alloc_card(struct mmc_host *host)
{
    struct mmc_card *card = NULL;

    if (!host->card_list) {
        host->card_list = malloc(sizeof(struct mmc_card_list));
        if (!host->card_list) {
            mmc_err("host allocate mmc card fail!");
            return NULL;
        }
        memset(host->card_list, 0, sizeof(struct mmc_card_list));
        card = &(host->card_list->card);
        if (mmc_mutex_init(&card->lock))
            goto err;
        card->host = host;
        host->card_cur = card;
        mmc_reset_card(card);
    }
    else {
        /* TODO: add more cards */
        card = &(host->card_list->card);
    }
    mmc_trace(2, "host %d card addr is 0x%x", host->idx, card);
    return card;

err:
    mmc_err("host allocate mmc card fail!");
    if (host->card_list)
        free(host->card_list);
    host->card_list = NULL;
    return NULL;
}

void mmc_reset_card(struct mmc_card *card)
{
    card->type = CARD_TYPE_UNKNOW;
    card->capacity = 0;
    card->state.state_data = 0;
    memset(&card->card_reg, 0, sizeof(struct mmc_card_reg));

    card->erase_size = 0;
    card->sdio_func_num = 0;

    card->bus_speed_mode = SD_MODE_UHS_SDR12;
    card->iocfg.clock = card->host->freq_default;    /* default */
    card->iocfg.vdd = VDD_3V3;    /* default */
    card->iocfg.power_mode = POWER_ON;    /* default */
    card->iocfg.chip_select = CS_DONTCARE;  /* default */
    card->iocfg.bus_width = BUS_WIDTH_1;    /* default */
    card->iocfg.timing = TIMING_MMC_DS;    /* default */
}

void mmc_free_card(struct mmc_host *host)
{
    struct mmc_card *card = host->card_cur;
    /* delete sdio device from device */
    if (is_card_sdio(card) || is_card_combo(card))
        sdio_delete_func(card);
    if (host->card_list)
        free(host->card_list);
    host->card_list = NULL;
    host->card_cur = NULL;
}

/* FIXME: */
int mmc_add_card(struct mmc_card *card)
{
    int ret = 0;
    int cardno = 0;
    const char *type;
    switch (get_card_type(card)) {
        case CARD_TYPE_MMC:
            type = "MMC";
            break;
        case CARD_TYPE_SD:
            type = "SD";
            if (is_card_blkaddr(card)) {
                if (is_card_ext_capacity(card))
                    type = "SDXC";
                else
                    type = "SDHC";
            }
            break;
        case CARD_TYPE_SDIO:
            type = "SDIO";
            break;
        case CARD_TYPE_COMBO:
            type = "SD-combo";
            if (is_card_blkaddr(card))
                type = "SDHC-combo";
            break;
        default:
            type = "unknow";
            break;
    }
    set_card_present(card);
    /* FIXME: */
    card->card_idx = card->host->idx;
    mmc_printf(MMC_PRINTF_INFO, "new %s%s%s%s card at address %04x\n",
            is_card_uhs(card) ? "ultra high speed " :
            (is_card_highspeed(card) ? "high speed " : ""),
            (is_card_hs200(card) ? "HS200 " : ""),
            is_card_ddr_mode(card) ? "DDR " : "",
            type, card->card_reg.rca);

    if (!card->old_card) {
    ret = mmc_block_init(card);
    if (ret < 0)
        return ret;
    }
    return 0;
}

void mmc_del_card(struct mmc_host *host)
{
    struct mmc_card *card = host->card_cur;

    if (!is_card_present(card)) {
        mmc_trace(3,"err:card_cur = 0x%x,card state is wrong!\n", host->card_cur);
        return;
    }
    clr_card_present(card);
    mmc_block_deinit(card);
    mmc_mutex_delete(card->lock);

    /* delete sdio device from device */

    mmc_free_card(host);
}

/* end of file card.c */
