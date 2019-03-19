/* himci_sus_res.c begin */

#include "himci.h"
#include "mmc_core.h"

int mmc_host_suspend(void *data)
{
    struct mmc_suspend_resume *mmc_sus_res = NULL;

    struct mmc_host *mmc = NULL;
    struct mmc_card *card = NULL;
    struct himci_host *host = NULL;
    int mmc_num = 0;
    int mmc_idx = 0;
    int mmc_need_detect = 0;

    mmc_printf(0, "himci is suspending...");
    if (data) {
        mmc_sus_res = (struct mmc_suspend_resume*)data;
        mmc_num = mmc_sus_res->mmc_max_num;
    } else {
        mmc_num = get_mmc_max_num();
    }

    for(; mmc_idx < mmc_num; mmc_idx++) {
        mmc = get_mmc_host(mmc_idx);
        if (!mmc)
            continue;
        set_mmc_dis_detect(mmc, true);
        host = mmc->priv;
        card = mmc->card_cur;
        mmc_need_detect = 0;
        if (mmc_sus_res) {
            if (MMC_DO_DETECT == (mmc_sus_res->flags[mmc_idx]))
                mmc_need_detect = 1;
        } else {
            if (card && (!is_card_sdio(card)))
                mmc_need_detect = 1;
        }
#if 0
        if (card && mmc_need_detect) {
            //clr_card_present(card);
            mmc_del_card(mmc);
            host->card_status = CARD_UNPLUGED;
        }
#endif
    }
    mmc_printf(0, "himci is suspended.");
    return 0;
}

int mmc_host_resume(void *data)
{
    struct mmc_suspend_resume *mmc_sus_res = NULL;

    struct mmc_host *mmc = NULL;
    struct mmc_card *card = NULL;
    struct himci_host *host = NULL;
    int mmc_idx = 0;
    int mmc_need_detect = 0;
    int mmc_num = 0;

    mmc_printf(0, "himci is resuming...");
    if (data) {
        mmc_sus_res = (struct mmc_suspend_resume*)data;
        mmc_num = mmc_sus_res->mmc_max_num;
    } else {
        mmc_num = get_mmc_max_num();
    }

    for (; mmc_idx < mmc_num; mmc_idx++) {
        mmc = get_mmc_host(mmc_idx);
        if (!mmc)
            continue;
        host = (struct himci_host *)mmc->priv;
        card = mmc->card_cur;
        /* here enable irq vector */
        hal_interrupt_unmask((int)host->irq_num);
        if (mmc_sus_res){
            if (MMC_UNDO_DETECT == (mmc_sus_res->flags[mmc_idx]))
                mmc_need_detect = 0;
        } else {
            if (!card)
                mmc_need_detect = 1;
            else {
                if (is_card_sdio(card))
                mmc_need_detect = 0;
                else { //SD or eMMC
                    mmc_del_card(mmc);
                    host->card_status = CARD_UNPLUGED;
            mmc_need_detect = 1;
                }
            }
        }
        if (mmc_need_detect) {
            int ret = 0;
            mmc_thread task_id;
            set_mmc_dis_detect(mmc, false);
            ret = mmc_thread_create(5,
                    hi_mci_pre_detect_card,
                    HIMCI_STACKSIZE,
                    (void *)host,
                    "himci_Pre_detect",
                    &task_id);
            if (ret) {
                mmc_trace(5,"himci_Pre_detect create fail");
            }
        } else {
            card = mmc->card_cur;
            mmc_mutex_lock(host->thread_mutex, MMC_MUTEX_WAIT_FOREVER);
            mmc_hw_init(mmc);
            mmc_set_power_mode(mmc, card->iocfg.power_mode);
            mmc_set_clock(mmc, card->iocfg.clock);
            mmc_set_timing(mmc, card->iocfg.timing);
            mmc_set_bus_width(mmc, card->iocfg.bus_width);
            switch(card->iocfg.vdd) {
                case VDD_3V3:
                    mmc_voltage_switch(mmc, SIGNAL_VOLT_3V3);
                    break;
                case VDD_1V8:
                    mmc_voltage_switch(mmc, SIGNAL_VOLT_1V8);
                    break;
                default:
                    mmc_printf(0, "err vdd ");
                    break;
            }
            mmc_mutex_unlock(host->thread_mutex);
        }
    }
    mmc_printf(0, "himci is resumed");
    return 0;
}

/* himci_sus_res.c */
