/* host.c begin */
#include "mmc_core.h"

#define DEFAULT_FREQ        400000

/* host point storage */
static struct mmc_host **mmc_list;

int mmc_alloc_mmc_list(void)
{
    void * mmc_list_head = NULL;
    uint32_t mmc_max_num = get_mmc_max_num();

    if (!mmc_max_num) {
        mmc_trace(5, "mmc host num can't be zero");
        return -EINVAL;
    }
    mmc_list_head = (void *)malloc(mmc_max_num * sizeof(void *));
    if (!mmc_list_head) {
        mmc_trace(5, "No memory for mmc host list");
        return -ENOMEM;
    }
    memset(mmc_list_head, 0, mmc_max_num * sizeof(void *));
    mmc_list =(struct mmc_host **)mmc_list_head;
    return 0;
}

void mmc_free_mmc_list(void)
{
    free(mmc_list);
    mmc_list = NULL;
}

int set_mmc_host(struct mmc_host *host, uint32_t mmc_idx)
{
    uint32_t mmc_max_num = get_mmc_max_num();
    if (mmc_idx > (mmc_max_num - 1)) {
        mmc_trace(5, "mmc index shouldn't be more than %d", mmc_max_num - 1);
        return -EINVAL;
    }
    if (!mmc_list) {
        mmc_trace(5, "mmc list hasn't be allocated!");
        return -EINVAL;
    }
    mmc_list[mmc_idx] = host;

    return 0;
}

struct mmc_host *get_mmc_host(uint32_t mmc_idx)
{
    uint32_t mmc_max_num = get_mmc_max_num();
    if (mmc_idx > (mmc_max_num - 1)) {
        mmc_trace(5, "mmc index shouldn't be more than %d", mmc_max_num - 1);
        return NULL;
    }
    return mmc_list[mmc_idx];
}

static void mmc_power_up(struct mmc_host *host)
{
    mmc_set_clock(host, 0);
    mmc_set_power_mode(host, POWER_UP);
    mmc_set_bus_width(host, BUS_WIDTH_1);
    mmc_set_timing(host, TIMING_MMC_DS);
    mmc_delay_ms(10);
    mmc_set_clock(host, host->freq_default);
    mmc_set_power_mode(host, POWER_ON);
    /*
     ** This delay must be at least 74 clock sizes, or 1 ms, or the
     ** time required to reach a stable voltage.
     * */
    mmc_delay_ms(10);
}

static void mmc_power_off(struct mmc_host *host)
{
    //host->clock = 0;
    mmc_set_clock(host, 0);
    mmc_set_power_mode(host, POWER_OFF);
    mmc_set_bus_width(host, BUS_WIDTH_1);
}


int mmc_do_detect(struct mmc_host *host)
{
    const unsigned int freq = DEFAULT_FREQ;
    struct mmc_card *card = NULL;
    int ret = 0;

    if (host->is_mmc_detect_dis)
        return -EINVAL;
    host->freq_default = freq;
    host->volt_default = SIGNAL_VOLT_3V3;

    card = mmc_alloc_card(host);
    if (!card) {
        mmc_trace(5, "No memory for mmc_card");
        return -ENOMEM;
    }
    mmc_power_up(host);
    mmc_hw_reset(host);
    (void)mmc_voltage_switch(host, SIGNAL_VOLT_3V3);

    mmc_priv_fn(card);

    (void)sdio_reset(card);
    mmc_idle_card(card);
    (void)sd_send_if_cond(card, host->ocr_default.ocr_data);

    if (!mmc_assume_sdio(card))
        goto out;
    if (!mmc_assume_sd(card))
        goto out;
    if (!mmc_assume_mmc(card))
        goto out;

    mmc_free_card(host);
    mmc_power_off(host);

    return -EIO;
out:
    return 0;
}

int card_reset(struct mmc_card *card)
{
    struct mmc_host * mmc = card->host;

    /*reset host*/
    mmc_hw_init(mmc);

    mmc->freq_default = 400000;
    mmc->volt_default = SIGNAL_VOLT_3V3;
    card->iocfg.clock = 400000;

    mmc_reset_card(card);
    card->old_card = 1;

    mmc_power_up(mmc);
    mmc_voltage_switch(mmc, SIGNAL_VOLT_3V3);
    mmc_idle_card(card);

    sd_send_if_cond(card, mmc->ocr_default.ocr_data);

    return sd_card_reset(card);
}
/* end of file host.c */
