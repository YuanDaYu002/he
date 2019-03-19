/* himci.c begin */

#include "mmc_core.h"
#include "himci.h"
/*
#define HIMCI_PAGE_SIZE   4096
*/

#ifdef LOSCFG_DRIVERS_MMC_SPEEDUP
void himci_prep_entire(struct himci_host *host, struct mmc_request *mrq);
void himci_err_check(struct himci_host *host, unsigned int qstat,
        unsigned int rstat, unsigned int *cmd_err, unsigned int *data_err);
int himci_send_stop(struct mmc_host *host);
unsigned int himci_get_stable_rd_pos(struct himci_host *host,
        unsigned int qstat, unsigned int rstat);
#endif
/* time calcute */
#ifdef MMC_POWERUP_TIME_CAL
unsigned long long  sd_time_s = 0;
unsigned long long  sd_time_e = 0;
#endif

extern int himci_proc_init(void);
static unsigned int retry_count = MAX_RETRY_COUNT;
static unsigned int request_timeout = HI_MCI_REQUEST_TIMEOUT;

/* Tuning Block Pattern UHS-I */
static const uint8_t tuning_blk_4bit[] = {
    0xFF, 0x0F, 0xFF, 0x00, 0xFF, 0xCC, 0xC3, 0xCC,
    0xC3, 0x3C, 0xCC, 0xFF, 0xFE, 0xFF, 0xFE, 0xEF,
    0xFF, 0xDF, 0xFF, 0xDD, 0xFF, 0xFB, 0xFF, 0xFB,
    0xBF, 0xFF, 0x7F, 0xFF, 0x77, 0xF7, 0xBD, 0xEF,
    0xFF, 0xF0, 0xFF, 0xF0, 0x0F, 0xFC, 0xCC, 0x3C,
    0xCC, 0x33, 0xCC, 0xCF, 0xFF, 0xEF, 0xFF, 0xEE,
    0xFF, 0xFD, 0xFF, 0xFD, 0xDF, 0xFF, 0xBF, 0xFF,
    0xBB, 0xFF, 0xF7, 0xFF, 0xF7, 0x7f, 0x7B, 0xDE,
};

/* Tuning block pattern for 8 bit mode for HS200 */
static const uint8_t tuning_blk_8bit[] = {
    0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
    0xFF, 0xFF, 0xCC, 0xCC, 0xCC, 0x33, 0xCC, 0xCC,
    0xCC, 0x33, 0x33, 0xCC, 0xCC, 0xCC, 0xFF, 0xFF,
    0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE, 0xFF,
    0xFF, 0xFF, 0xDD, 0xFF, 0xFF, 0xFF, 0xDD, 0xDD,
    0xFF, 0xFF, 0xFF, 0xBB, 0xFF, 0xFF, 0xFF, 0xBB,
    0xBB, 0xFF, 0xFF, 0xFF, 0x77, 0xFF, 0xFF, 0xFF,
    0x77, 0x77, 0xFF, 0x77, 0xBB, 0xDD, 0xEE, 0xFF,
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00,
    0x00, 0xFF, 0xFF, 0xCC, 0xCC, 0xCC, 0x33, 0xCC,
    0xCC, 0xCC, 0x33, 0x33, 0xCC, 0xCC, 0xCC, 0xFF,
    0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE, 0xEE,
    0xFF, 0xFF, 0xFF, 0xDD, 0xFF, 0xFF, 0xFF, 0xDD,
    0xDD, 0xFF, 0xFF, 0xFF, 0xBB, 0xFF, 0xFF, 0xFF,
    0xBB, 0xBB, 0xFF, 0xFF, 0xFF, 0x77, 0xFF, 0xFF,
    0xFF, 0x77, 0x77, 0xFF, 0x77, 0xBB, 0xDD, 0xEE,
};

struct tuning_blk_info {
    uint8_t *blk;
    unsigned int blk_size;
};

struct squirk_mmc_dev {
    int host_id;
    void *data;
    void (*preinit)(struct mmc_card *card, void *data);
    unsigned int detect_by_1V8;
#define DETECT_BY_1V8_ENABLE        1
#define DETECT_BY_1V8_DISABLE       0
};

static struct squirk_mmc_dev g_sq_dev[MAX_MMC_NUM] = {0};
int g_sdioirq_disable = 0;

/* local function declaration */
static irqreturn_t hisd_irq(int irq, void *data);
static void sd_mmc_thread(UINT32 p1, UINT32 p2, UINT32 p3, UINT32 p4);
static void himci_read_response(struct himci_host *host,
                struct mmc_cmd* cmd);
int mmc_detect_by_1V8_enable(unsigned int id)
{
    if (id >= MAX_MMC_NUM) {
        mmc_err("Id: invalid argument!\n");
        return -1;
    }

    mmc_trace(3,"host %d: detect by 1.8v !\n",id);
    g_sq_dev[id].detect_by_1V8 = DETECT_BY_1V8_ENABLE;
    return 0;
}

int mmc_priv_fn_register(unsigned int id, void *data, void *fn(struct mmc_card *card, void *data))
{
   if (id >= MAX_MMC_NUM) {
      mmc_err("Id: invalid argument!\n");
      return -1;
   }

   g_sq_dev[id].host_id = id;
   g_sq_dev[id].data = data;
   g_sq_dev[id].preinit = (void *)fn;
   return 0;
}

void mmc_priv_fn(struct mmc_card *card)
{
    struct mmc_host *host = card->host;
    struct squirk_mmc_dev * g_sq_devp = &g_sq_dev[host->idx];

	mmc_trace(2, "mmc priv fn call!");
    if (g_sq_devp->host_id == host->idx && g_sq_devp->preinit )
        g_sq_devp->preinit(card, g_sq_devp->data);
}

void sdio_irq_enable(int enable)
{
    g_sdioirq_disable = 0;
    if (!enable)
        g_sdioirq_disable = 1;
}

void mmc_dma_cache_inv(void *addr, unsigned int size)
{
    unsigned int start = (unsigned int)addr & ~(CACHE_ALIGNED_SIZE - 1);
    unsigned int end = (unsigned int)addr + size;

    end = ALIGN(end, CACHE_ALIGNED_SIZE);

    dma_cache_inv(start, end);
}

void mmc_dma_cache_clean(void *addr, unsigned int size)
{
    unsigned int start = (unsigned int)addr & ~(CACHE_ALIGNED_SIZE - 1);
    unsigned int end = (unsigned int)addr + size;

    end = ALIGN(end, CACHE_ALIGNED_SIZE);

    dma_cache_clean(start, end);
}

/* reset MMC host controler */
static void hi_mci_sys_reset(struct himci_host *host)
{
    unsigned int reg_value;

    mmc_trace(2, "reset");
    //hi_mci_ctr_reset(host);
    //mmc_delay_us(50);
    //hi_mci_ctr_undo_reset(host);

    reg_value = himci_readl(host->base + MCI_BMOD);
    reg_value |= BMOD_SWR;
    himci_writel(reg_value, host->base + MCI_BMOD);
    mmc_delay_us(10);
    reg_value = himci_readl(host->base + MCI_BMOD);
    reg_value |= MCI_BMOD_VALUE;
    himci_writel(reg_value, host->base + MCI_BMOD);

    reg_value = himci_readl(host->base + MCI_CTRL);
    reg_value |=  CTRL_RESET | FIFO_RESET | DMA_RESET;
    himci_writel(reg_value, host->base + MCI_CTRL);
}

static unsigned int hi_mci_sys_card_detect(struct himci_host *host)
{
    unsigned int card_status;
    card_status = readl((unsigned int)host->base + MCI_CDETECT);

    if (host->mmc->caps.bits.cap_nonremovable)  /* suppor emmc */
        card_status = 0;
    return card_status & HIMCI_CARD0;
}
/*
static void hi_mci_sys_undo_reset(struct himci_host *host)
{
    unsigned long flags;

    mmc_trace(2, "undo reset");

    hi_mci_ctr_undo_reset(host);
}
*/
void hi_mci_ctrl_power(struct himci_host *host, enum power_status flag,
        unsigned int force)
{
    unsigned int reg_value;
    mmc_trace(2, "begin");

    if (host->power_status != flag || force == FORCE_ENABLE) {
        //if (flag == HOST_POWER_OFF) {
        //    reg_value = himci_readl(host->base + MCI_RESET_N);
        //    reg_value &= ~(MMC_RST_N << host->port);
        //    himci_writel(reg_value, host->base + MCI_RESET_N);


        reg_value = himci_readl(host->base + MCI_PWREN);
        if (flag == HOST_POWER_OFF)
            reg_value &= ~(0x1 << host->port);
        else
            reg_value |= (0x1 << host->port);

        himci_writel(reg_value, host->base + MCI_PWREN);

        //if (flag == HOST_POWER_ON ) {
        //    reg_value = himci_readl(host->base + MCI_RESET_N);
        //    reg_value |= (MMC_RST_N << host->port);
        //    himci_writel(reg_value, host->base + MCI_RESET_N);
        //}
        mmc_delay_ms(50);
        host->power_status = flag;
    }
}
/* mmc controller initialize */
static void hi_mci_host_init(struct himci_host *host)
{
    unsigned int tmp_reg = 0;
    unsigned long flags = 0;

    mmc_trace(MMC_TRACE_INFO, "begin");
    mmc_assert(host);

    host->error_count = 0;

    hi_mci_sys_reset(host);

    hi_mci_ctrl_power(host, HOST_POWER_OFF, FORCE_ENABLE);
    /* host power on */
    hi_mci_ctrl_power(host, HOST_POWER_ON, FORCE_ENABLE);

    tmp_reg = ((DRV_PHASE_SHIFT << CLK_DRV_PHS_OFFSET)
           | (SMPL_PHASE_SHIFT << CLK_SMPL_PHS_OFFSET));
    himci_writel(tmp_reg, host->base + MCI_UHS_EXT);

    himci_writel(READ_THRESHOLD_SIZE, host->base + MCI_CARDTHRCTL);

    /* clear MMC host intr */
    himci_writel(ALL_INT_CLR, host->base + MCI_RINTSTS);

    /* MASK MMC host intr */
    tmp_reg = himci_readl(host->base + MCI_INTMASK);
    tmp_reg &= ~ALL_INT_MASK;
    //tmp_reg |= DTO_INT_MASK; /*pf fix flag*/
    himci_writel(tmp_reg, host->base + MCI_INTMASK);

    /* enable inner DMA mode and close intr of MMC host controler */
    tmp_reg = himci_readl(host->base + MCI_CTRL);
    tmp_reg &= ~INTR_EN;
    tmp_reg |= USE_INTERNAL_DMA | INTR_EN;
    himci_writel(tmp_reg, host->base + MCI_CTRL);

    /* set timeout param */
    himci_writel(DATA_TIMEOUT | RESPONSE_TIMEOUT, host->base + MCI_TIMEOUT);

    /* set FIFO param */
    tmp_reg = 0;
    tmp_reg |= BURST_SIZE | RX_WMARK | TX_WMARK;
    himci_writel(tmp_reg, host->base + MCI_FIFOTH);
#ifdef LOSCFG_DRIVERS_MMC_SPEEDUP
    mmc_printf(0, "using speed up mode on hi3519");
    tmp_reg = himci_readl(host->base + MCI_IDSTS);
    tmp_reg |= ALL_ADMA_INT_CLR;
    himci_writel(tmp_reg, host->base + MCI_IDSTS);
    /* ADMA3: disable ADMA Func */
    himci_writel(0x0, host->base + ADMA_CTRL);
    /* ADMA3: set entire des addr and quene deepth  */
    himci_writel((unsigned long)host->dma_vaddr, host->base + ADMA_Q_ADDR); /* */
    himci_writel(ADMA_QUEUE_DEEPTH, host->base + ADMA_Q_DEEPTH);
    /* ADMA3: reset queue read/write ptr */
    tmp_reg = himci_readl(host->base + ADMA_CTRL);
    tmp_reg |= RDPTR_MOD_EN;
    himci_writel(tmp_reg, host->base + ADMA_CTRL);
    himci_writel(0x0, host->base + ADMA_Q_RDPTR);
    himci_writel(0x0, host->base + ADMA_Q_WRPTR);
    tmp_reg = himci_readl(host->base + ADMA_CTRL);
    tmp_reg &= ~RDPTR_MOD_EN;
    himci_writel(tmp_reg, host->base + ADMA_CTRL);
    /* ADMA3: set queue timeout value */
    himci_writel(0x30000000, host->base + ADMA_Q_TO);
    /* ADMA3: enable ADMA intr */
    tmp_reg = (CMD_LOCK_ERR | OWNBIT_ERR | QUEUE_OVERFLOW
            | RESP_CHECK_ERR | PACKET_INT | PACKET_TO_INT
            | AUTO_STOP_ERR | QUEUE_FULL | CES);
    himci_writel(tmp_reg, host->base + MCI_IDINTEN);
    /* ADMA3: clear ADMA3 intr */
    tmp_reg = (CMD_LOCK_ERR | OWNBIT_ERR | QUEUE_OVERFLOW
            | RESP_CHECK_ERR | PACKET_INT | PACKET_TO_INT
            | AUTO_STOP_ERR | QUEUE_FULL | QUEUE_EMPTY | CES);
    himci_writel(tmp_reg, host->base + MCI_IDSTS);
    /* ADMA3: enable ADMA Func */
    tmp_reg = PACKET_INT_EN | ADMA3_EN;
    himci_writel(tmp_reg, host->base + ADMA_CTRL);
    //himci_idma_start(host);
    tmp_reg = himci_readl(host->base + MCI_BMOD);
    tmp_reg |= BMOD_DMA_EN;
    himci_writel(tmp_reg, host->base + MCI_BMOD);

    host->mmc->status = MMC_HOST_OK;
#endif
#if 0
    /* set hs400 dll clk reset and unreset */
    tmp_reg = himci_readl(PERI_CRG49);
    tmp_reg |= SDIO2_DLL_SRST_REQ;
    himci_writel(tmp_reg, PERI_CRG49);

    tmp_reg = himci_readl(PERI_CRG49);
    tmp_reg &= ~SDIO2_DLL_SRST_REQ;
    himci_writel(tmp_reg, PERI_CRG49);

    /*set hs400 dll clk enable*/
    tmp_reg = himci_readl(PERI_CRG49);
    tmp_reg |= SDIO2_DLL_CKEN;
    himci_writel(tmp_reg, PERI_CRG49);

    himci_writel(0x10, MISC_DLL);
#endif
}

/* mmc system init */
static void hi_mci_sys_ctrl_init(struct himci_host *host)
{
    mmc_task_lock();
    hi_mci_detect_polarity_cfg(host->id);
    hi_mci_clock_cfg(host);
    hi_mci_soft_reset(host->id);
    mmc_task_unlock();

    hi_mci_pad_ctrl_cfg(host, SIGNAL_VOLT_3V3);
}

void himci_mmc_rescan(int slot)
{
    struct mmc_host *mmc = NULL;
    struct himci_host *host = NULL;

    if (slot >= MAX_MMC_NUM) {
        mmc_err("Id: invalid argument!\n");
        return;
    }
    mmc = get_mmc_host(slot);
    host = mmc->priv;
    mmc_mutex_lock(host->thread_mutex, MMC_MUTEX_WAIT_FOREVER);
    host->card_status = CARD_IGNORE;

    mmc_del_card(host->mmc);

    hi_mci_sys_ctrl_init(host);
    hi_mci_host_init(host);

    /* if no removeable, not printf */
    if (!(host->mmc->caps.bits.cap_nonremovable))
        mmc_printf(0, "host %d a card is pluged.", host->id);
    for (int i = 0; i < 5; i++) {
        if (!mmc_do_detect(host->mmc))
            break;
    }

    mmc_mutex_unlock(host->thread_mutex);
    return;
}

void hi_mci_pre_detect_card(struct himci_host *host)
{
    unsigned int curr_status;
    unsigned int card_status;
    //int i;
    //unsigned int detect_retry_count = 0;

    mmc_trace(0, "host %d is pre detecting...", host->id);
    mmc_mutex_lock(host->thread_mutex, MMC_MUTEX_WAIT_FOREVER);
    if (host->card_status == CARD_IGNORE) {
        mmc_mutex_unlock(host->thread_mutex);
        return;
    }

    curr_status = hi_mci_sys_card_detect(host);
    if (curr_status != host->card_status) {
        host->card_status = curr_status;
        if (curr_status != CARD_UNPLUGED) {
            hi_mci_sys_ctrl_init(host);
            hi_mci_host_init(host);
            /* if no removeable, not printf */
            if (!(host->mmc->caps.bits.cap_nonremovable))
                mmc_printf(0, "host %d a card is pluged.", host->id);
            for (int i = 0; i < 5; i++) {
                if (!mmc_do_detect(host->mmc))
                    break;
            }
        }
    }
    mmc_mutex_unlock(host->thread_mutex);
}


static void hi_mci_detect_card(struct himci_host *host)
{
    unsigned int curr_status;
    unsigned int status[3];
    int i;
    unsigned int detect_retry_count = 0;

    mmc_trace(0, "host %d is detecting...", host->id);
    mmc_mutex_lock(host->thread_mutex, MMC_MUTEX_WAIT_FOREVER);

    if (host->card_status == CARD_IGNORE) {
        mmc_mutex_unlock(host->thread_mutex);
        return;
    }

    while (1) {
        for (i = 0; i < 3; i++) {
            status[i] = hi_mci_sys_card_detect(host);
            mmc_sleep_ms(10);
        }
        if ((status[0] == status[1])
                && (status[0] == status[2]))
            break;
        detect_retry_count++;
        if (detect_retry_count >= retry_count) {
            mmc_err("this is a dithering,card detect error!");
            return;
        }
    }
    curr_status = status[0];
    if (curr_status != host->card_status) {
        host->card_status = curr_status;
        if (curr_status != CARD_UNPLUGED) {
            hi_mci_sys_ctrl_init(host);
            hi_mci_host_init(host);
            /* if no removeable, not printf */
            if (!(host->mmc->caps.bits.cap_nonremovable))
                mmc_printf(0, "host %d a card is pluged.", host->id);
            for (int i = 0; i < 5; i++) {
                if (!mmc_do_detect(host->mmc))
                    break;
            }
        } else {
            mmc_del_card(host->mmc);
#ifdef LOSCFG_DRIVERS_MMC_SPEEDUP
            host->mmc->status = MMC_HOST_ERR;
#endif
            /* if no removeable, not printf */
            if (!(host->mmc->caps.bits.cap_nonremovable))
                mmc_printf(5, "host %d card is unpluged.", host->id);
        }
    }
    mmc_mutex_unlock(host->thread_mutex);
}

static int hi_mci_wait_cmd(struct himci_host *host)
{
    int wait_retry_count = 0;
    unsigned int reg_data = 0;
    unsigned long flags = 0;

    mmc_assert(host);

    while (1) {
        /*
         * Check if CMD::start_cmd bit is clear.
         * start_cmd = 0 means MMC Host controller has loaded registers
         * and next command can be loaded in.
         */
        reg_data = himci_readl(host->base + MCI_CMD);
        if ((reg_data & START_CMD) == 0)
            return 0;

        /* Check if Raw_Intr_Status::HLE bit is set. */
        mmc_irq_lock(&flags);
        reg_data = himci_readl(host->base + MCI_RINTSTS);
        if (reg_data & HLE_INT_STATUS) {
            reg_data |= HLE_INT_STATUS;
            himci_writel(reg_data, host->base + MCI_RINTSTS);
            mmc_irq_unlock(flags);

            mmc_trace(5, "Other CMD is running,"
                    "please operate cmd again!");
            return 1;
        }
        mmc_irq_unlock(flags);
        mmc_delay_us(100);

        /* Check if number of retries for this are over. */
        wait_retry_count++;
        if (wait_retry_count >= retry_count) {
            mmc_err("wait cmd[%u] complete is timeout!", host->cmd->cmd_code);
            return -1;
        }
    }
}


static void hi_mci_idma_start(struct himci_host *host)
{
    unsigned int tmp;

    himci_writel(host->dma_paddr, host->base + MCI_DBADDR);
    tmp = himci_readl(host->base + MCI_BMOD);
    tmp |= BMOD_DMA_EN;
    himci_writel(tmp, host->base + MCI_BMOD);
}

static void hi_mci_idma_stop(struct himci_host *host)
{
    unsigned int tmp_reg;

    tmp_reg = himci_readl(host->base + MCI_BMOD);
    tmp_reg &= ~BMOD_DMA_EN;
    himci_writel(tmp_reg, host->base + MCI_BMOD);
}

static void hi_mci_control_cclk(struct himci_host* host, unsigned int flag)
{
    unsigned int reg;
    union cmd_arg_s cmd_reg;
    unsigned int port = 0;
    mmc_assert(host);
    port = host->port;

    reg = himci_readl(host->base + MCI_CLKENA);
    if (flag == ENABLE)
        reg |= (CCLK_ENABLE << port);
    else {
        reg &= ~(CCLK_ENABLE << port);
        //FIX wifi bug
        //reg |= (0x10000 << port);
    }
    himci_writel(reg, host->base + MCI_CLKENA);

    cmd_reg.cmd_arg = himci_readl(host->base + MCI_CMD);

    cmd_reg.bits.start_cmd = 1;
    cmd_reg.bits.card_number = port;
    cmd_reg.bits.cmd_index = 0;
    cmd_reg.bits.data_transfer_expected = 0;

    cmd_reg.bits.update_clk_reg_only = 1;
    himci_writel(cmd_reg.cmd_arg, host->base + MCI_CMD);

    if (hi_mci_wait_cmd(host) != 0)
        mmc_trace(3, "disable or enable clk is timeout!");
}

static void hi_mci_set_cclk(struct himci_host* host, unsigned int cclk)
{
    unsigned int reg_value;
    unsigned long flags = 0;
    union cmd_arg_s clk_cmd;
    mmc_assert(host);
    mmc_assert(cclk);

    mmc_task_lock();
    reg_value = hi_mci_clk_div(host, cclk);
    mmc_task_unlock();

    himci_writel(reg_value << (host->port*8), host->base + MCI_CLKDIV);

    clk_cmd.cmd_arg = himci_readl(host->base + MCI_CMD);
    clk_cmd.bits.start_cmd = 1;
    clk_cmd.bits.card_number = host->port;
    clk_cmd.bits.update_clk_reg_only = 1;
    clk_cmd.bits.cmd_index = 0;
    clk_cmd.bits.data_transfer_expected = 0;
    himci_writel(clk_cmd.cmd_arg, host->base + MCI_CMD);

    if (hi_mci_wait_cmd(host) != 0)
        mmc_trace(3, "set card clk divider is failed!");
}

/**********************************************
 *1: card readonly
 *0: card read/write
 ***********************************************/
static unsigned int hi_mci_ctrl_card_readonly(struct himci_host *host)
{
    unsigned int card_value = himci_readl(host->base + MCI_WRTPRT);
    return card_value & HIMCI_CARD0;
}

void mmc_set_bus_width(struct mmc_host *mmc, enum mmc_bus_width width)
{
    struct himci_host *host = mmc->priv;
    unsigned int reg;

    mmc_trace(3, "bus_width = %d ", (width==0)?1:((width==2)?4:8));
    reg = himci_readl(host->base + MCI_CTYPE);
    reg &= ~((CARD_WIDTH_0 | CARD_WIDTH_1) << host->port);

    if (width == BUS_WIDTH_8) {
        reg |= (CARD_WIDTH_0 << host->port);
        himci_writel(reg, host->base + MCI_CTYPE);
    } else if (width == BUS_WIDTH_4) {
        reg |= (CARD_WIDTH_1 << host->port);
        himci_writel(reg, host->base + MCI_CTYPE);
    } else {
        himci_writel(reg, host->base + MCI_CTYPE);
    }
}

void mmc_set_bus_mode(struct mmc_host *mmc, enum bus_mode mode)
{
    /* nothing to be done! */
}

/* get card
 * return 0, card still detected.
 * return 1, card removed. */
int mmc_get_card_detect(struct mmc_host *mmc)
{
    struct himci_host *host = mmc->priv;
    unsigned int card_status = 0;
    card_status = hi_mci_sys_card_detect(host);
    if (card_status == CARD_UNPLUGED)
        return 1;
    else
        return 0;
}

/* init host*/
void mmc_hw_init(struct mmc_host *mmc)
{
    struct himci_host *host = mmc->priv;
    hi_mci_sys_ctrl_init(host);
    hi_mci_host_init(host);
}

uint32_t get_mmc_max_num(void)
{
    return MAX_MMC_NUM;
}

void mmc_set_timing(struct mmc_host *mmc, enum mmc_bus_timing timing)
{
    struct himci_host *host = mmc->priv;
    int reg;

    mmc_trace(3, "timing = %d ", timing);
    reg = himci_readl(host->base + MCI_UHS_REG);

    /* speed mode check ,if it is DDR50 set DDR mode */
    if (timing == TIMING_UHS_DDR50) {
        if(!((HI_SDXC_CTRL_DDR_REG << host->port) & reg))
            reg |= (HI_SDXC_CTRL_DDR_REG << host->port);
    } else {
        if((HI_SDXC_CTRL_DDR_REG << host->port) & reg)
            reg &= ~(HI_SDXC_CTRL_DDR_REG << host->port);
    }
    himci_writel(reg, host->base + MCI_UHS_REG);
#ifdef CFG_PHASE_IN_TIMING
extern void himci_cfg_phase(struct mmc_host *mmc, enum mmc_bus_timing timing);
     himci_cfg_phase(mmc, timing);
#endif
}

void mmc_set_clock(struct mmc_host *mmc, unsigned int clock)
{
    struct himci_host *host = mmc->priv;

    if (clock > mmc->freq_max)
        clock = mmc->freq_max;

    mmc_trace(3, "mmc->clock = %d ", clock);
    hi_mci_control_cclk(host, DISABLE);
    if (clock) {
        hi_mci_set_cclk(host, clock);
        hi_mci_control_cclk(host, ENABLE);
    }
}

void mmc_set_power_mode(struct mmc_host *mmc, enum mmc_power_mode mode)
{
    struct himci_host *host = mmc->priv;
    unsigned int reg;

    mmc_trace(3, "power_mode = %d ", mode);
    if (POWER_OFF == mode) {
        reg = himci_readl(host->base + MCI_UHS_REG);
        reg &= ~(HI_SDXC_CTRL_VDD_180 << host->port);
        mmc_trace(3, "set voltage %d[addr 0x%x]", reg, MCI_UHS_REG);
        himci_writel(reg, host->base + MCI_UHS_REG);
        hi_mci_ctrl_power(host, HOST_POWER_OFF, FORCE_DISABLE);
    } else {
        /* POWER_UP or POWER_ON */
        hi_mci_ctrl_power(host, HOST_POWER_ON, FORCE_DISABLE);
    }
}

void mmc_set_driver_type(struct mmc_host *mmc, unsigned int type)
{
    /* nothing to be done! */
}
/* emmc spec */
void mmc_hw_reset(struct mmc_host *mmc)
{
    unsigned int reg_value;
    struct himci_host *host = mmc->priv;

    if(!(mmc->caps.bits.cap_hw_reset))
        return ;

    reg_value = himci_readl(host->base + MCI_RESET_N);
    reg_value &= ~(MMC_RST_N << host->port);
    himci_writel(reg_value, host->base + MCI_RESET_N);

    /* For eMMC, minimum is 1us but give it 10us for good measure */
    mmc_delay_us(10);

    reg_value = himci_readl(host->base + MCI_RESET_N);
    reg_value |= (MMC_RST_N << host->port);
    himci_writel(reg_value, host->base + MCI_RESET_N);

    mmc_delay_us(300);
}

int mmc_get_readonly(struct mmc_host *mmc)
{
    struct himci_host *host = mmc->priv;
    unsigned ret;

    ret = hi_mci_ctrl_card_readonly(host);

    return ret;
}

void mmc_set_sdio_irq(struct mmc_host *mmc, int enable)
{
    struct himci_host *host = mmc->priv;
    unsigned int reg_value;
    unsigned long flags = 0;
    mmc_irq_lock(&flags);
        reg_value = himci_readl(host->base + MCI_INTMASK);
    if (enable)
        reg_value |= SDIO_INT_MASK;
    else
        reg_value &= ~SDIO_INT_MASK;
    himci_writel(reg_value, host->base + MCI_INTMASK);
    mmc_irq_unlock(flags);
}

static int hi_mci_exec_cmd(struct himci_host *host, struct mmc_cmd *cmd,
    struct mmc_data *data)
{
    union cmd_arg_s  cmd_regs;
    unsigned int port = 0;

    mmc_trace(2, "begin");
    mmc_assert(host);
    mmc_assert(cmd);
    port = host->port;
    host->cmd = cmd;
#ifndef LOSCFG_DRIVERS_MMC_SPEEDUP
    himci_writel(cmd->arg, host->base + MCI_CMDARG);
    cmd_regs.cmd_arg = himci_readl(host->base + MCI_CMD);
#else
    struct himci_cmd_des *cmd_des;
    cmd_des = host->wr_cmd_des;
    cmd_des->arg = cmd->arg;
    cmd_regs.cmd_arg = DEFAULT_CMD_VALUE;
#endif
    mmc_trace(2, "cmd:%d, arg:0x%x, flag:0x%x", cmd->cmd_code, cmd->arg, cmd->resp_type);

    if (data) {
        cmd_regs.bits.data_transfer_expected = 1;
        if (data->data_flags & (MMC_DATA_WRITE | MMC_DATA_READ))
            cmd_regs.bits.transfer_mode = 0;

        if (data->data_flags & MMC_DATA_STREAM)
            cmd_regs.bits.transfer_mode = 1;

        if (data->data_flags & MMC_DATA_WRITE)
            cmd_regs.bits.read_write = 1;
        else if (data->data_flags & MMC_DATA_READ)
            cmd_regs.bits.read_write = 0;
    } else {
        cmd_regs.bits.data_transfer_expected = 0;
        cmd_regs.bits.transfer_mode = 0;
        cmd_regs.bits.read_write = 0;
    }

#ifdef CONFIG_SEND_AUTO_STOP
    if ((MMC_CMD_STOPENABLE((host->mrq->data))) && (!(host->is_tuning))) {
        cmd_regs.bits.send_auto_stop = 1;
    }
#else
    cmd_regs.bits.send_auto_stop = 0;
#endif
    if (cmd->cmd_code == STOP_TRANSMISSION) {
        cmd_regs.bits.stop_abort_cmd = 1;
        cmd_regs.bits.wait_prvdata_complete = 0;
    } else {
        cmd_regs.bits.stop_abort_cmd = 0;
        cmd_regs.bits.wait_prvdata_complete = 1;
    }

    switch (mmc_resp_type(cmd)) {
    case MMC_RESP_NONE:
        cmd_regs.bits.response_expect = 0;
        cmd_regs.bits.response_length = 0;
        cmd_regs.bits.check_response_crc = 0;
        break;
    case MMC_RESP_R1:
    case MMC_RESP_R1B:
        cmd_regs.bits.response_expect = 1;
        cmd_regs.bits.response_length = 0;
        cmd_regs.bits.check_response_crc = 1;
        break;
    case MMC_RESP_R2:
        cmd_regs.bits.response_expect = 1;
        cmd_regs.bits.response_length = 1;
        cmd_regs.bits.check_response_crc = 1;
        break;
    case MMC_RESP_R3:
    case MMC_RESP_R1 & (~RESP_CRC):
        cmd_regs.bits.response_expect = 1;
        cmd_regs.bits.response_length = 0;
        cmd_regs.bits.check_response_crc = 0;
        break;
    default:
        host->cmd->err = -EINVAL;
        mmc_err("hi_mci: unhandled response type %02x\n",
                mmc_resp_type(cmd));
        return -EINVAL;
    }
#ifndef LOSCFG_DRIVERS_MMC_SPEEDUP
    if (cmd->cmd_code == GO_IDLE_STATE)
        cmd_regs.bits.send_initialization = 1;
    else
        cmd_regs.bits.send_initialization = 0;
#else
    cmd_regs.bits.send_initialization = 1;
#endif

    /* CMD 11 check switch voltage */
    if (cmd->cmd_code == SD_SWITCH_VOLTAGE)
        cmd_regs.bits.volt_switch = 1;
    else
        cmd_regs.bits.volt_switch = 0;

    cmd_regs.bits.card_number = port;
    cmd_regs.bits.cmd_index = cmd->cmd_code;
    cmd_regs.bits.start_cmd = 1;
    cmd_regs.bits.update_clk_reg_only = 0;

#ifndef LOSCFG_DRIVERS_MMC_SPEEDUP
    himci_writel(DATA_INT_MASK, host->base + MCI_RINTSTS);
    himci_writel(cmd_regs.cmd_arg, host->base + MCI_CMD);
    mmc_trace(3,"cmd_reg 0x%x, val 0x%x\n",MCI_CMD, cmd_regs.cmd_arg);

    if (hi_mci_wait_cmd(host) != 0) {
        mmc_trace(5, "send card cmd is failed!");
        return -EINVAL;
    }
#else
    cmd_des->cmd = cmd_regs.cmd_arg;
    mmc_trace(3, "cmd_reg 0x%x, val 0x%x\n", MCI_CMD, cmd_regs.cmd_arg);
    mmc_trace(3, "cmd_des blk_sz = 0x%x, byte_cnt = 0x%x,arg = 0x%x, cmd= 0x%x\n",
            cmd_des->blk_sz, cmd_des->byte_cnt,cmd_des->arg, cmd_des->cmd);
#endif

    return 0;
}


static void hi_mci_cmd_done(struct himci_host *host, unsigned int stat)
{
    unsigned int i;
    struct mmc_cmd *cmd = NULL;

    mmc_trace(2, "begin");
    mmc_assert(host);
    cmd = host->cmd;
    mmc_assert(cmd);
#ifndef LOSCFG_DRIVERS_MMC_SPEEDUP
    for (i = 0; i < 4; i++) {
        if (mmc_resp_type(cmd) == MMC_RESP_R2) {
            cmd->resp[i] = himci_readl(host->base +
                    MCI_RESP3 - i * 0x4);
            /*
             * R2 must delay some time here ,when use UHI card,
             * need check why
             */
            mmc_delay_us(1000);
        } else
            cmd->resp[i] = himci_readl(host->base
                    + MCI_RESP0 + i * 0x4);
    }
#endif
    if (stat & RTO_INT_STATUS) {
        cmd->err = -ETIMEDOUT;
        mmc_trace(2, "irq cmd status stat = 0x%x is timeout error!",
                stat);
    } else if (stat & (RCRC_INT_STATUS | RE_INT_STATUS)) {
        cmd->err = -EILSEQ;
        mmc_trace(3, "irq cmd status stat = 0x%x is response error!",
                stat);
    }

    host->cmd = NULL;
}

static void hi_mci_data_done(struct himci_host *host, unsigned int stat)
{
    struct mmc_data *data = NULL;

    mmc_trace(2, "begin");
    mmc_assert(host);
    data = host->data;
    mmc_assert(data);

    if (stat & (HTO_INT_STATUS | DRTO_INT_STATUS)) {
        data->err = -ETIMEDOUT;
        mmc_trace(5, "hostbase:0x%x:irq data status stat = 0x%x is timeout error!",
                host->base, stat);
    } else if (stat & (EBE_INT_STATUS | SBE_INT_STATUS | FRUN_INT_STATUS
                | DCRC_INT_STATUS)) {
        data->err = -EILSEQ;
        mmc_trace(3, "hostbase:0x%x:irq data status stat = 0x%x is data error!",
                host->base, stat);
    }

    if (!data->err)
        data->bytes_xfered = data->blocks * data->blocksz;
    else
        data->bytes_xfered = 0;

    host->data = NULL;
}

static int hi_mci_wait_cmd_complete(struct himci_host *host)
{
    unsigned int cmd_retry_count = 0;
    unsigned long cmd_timeout;
    unsigned int cmd_irq_reg = 0;
    struct mmc_cmd *cmd = NULL;
    unsigned long flags = 0;

    mmc_trace(2, "begin");
    mmc_assert(host);
    cmd = host->cmd;
    mmc_assert(cmd);
    cmd_timeout = mmc_get_sys_ticks()+ request_timeout;
    while (1) {
        do {
            mmc_irq_lock(&flags);
            cmd_irq_reg = himci_readl(host->base + MCI_RINTSTS);
#ifndef LOSCFG_DRIVERS_MMC_SPEEDUP
            if (cmd_irq_reg & CD_INT_STATUS) {
                himci_writel((CD_INT_STATUS | RTO_INT_STATUS
                        | RCRC_INT_STATUS | RE_INT_STATUS),
                        host->base + MCI_RINTSTS);
                mmc_irq_unlock(flags);

                hi_mci_cmd_done(host, cmd_irq_reg);
                //mmc_trace(3, "cmd_retry_count:%d", cmd_retry_count);

                return 0;
            } else if (cmd_irq_reg & VOLT_SWITCH_INT_STATUS) {
                himci_writel(VOLT_SWITCH_INT_STATUS,
                        host->base + MCI_RINTSTS);
                mmc_irq_unlock(flags);
                hi_mci_cmd_done(host, cmd_irq_reg);
                return 0;
            }
#else
            if (cmd_irq_reg & VOLT_SWITCH_INT_STATUS) {
                himci_writel(VOLT_SWITCH_INT_STATUS,
                        host->base + MCI_RINTSTS);
                mmc_irq_unlock(flags);
                hi_mci_cmd_done(host, cmd_irq_reg);
                himci_writel(SDIO_INT_STATUS, host->base + MCI_RINTSTS);
                return 0;
            }
#endif
            mmc_irq_unlock(flags);
            cmd_retry_count++;
        } while (cmd_retry_count < retry_count);

        cmd_retry_count = 0;
        if (host->card_status == CARD_UNPLUGED) {
            mmc_trace(5, "card plugout");
            cmd->err = -EIO;
            return -EIO;
        }

        if (mmc_get_sys_ticks() > cmd_timeout) {
            unsigned int i = 0;
            for (i = 0; i < 4; i++) {
                cmd->resp[i] = himci_readl(host->base
                        + MCI_RESP0 + i * 0x4);
                mmc_err("voltage switch read MCI_RESP");
                mmc_err("%d : 0x%x\n", i, cmd->resp[i]);
            }
            cmd->err = -ETIMEDOUT;
            mmc_trace(5, "wait cmd request complete is timeout!");
            return -ETIMEDOUT;
        }
        mmc_thread_sched();
    }
}

/*
 * designware support send stop command automatically when
 * read or wirte multi blocks
 */
#ifdef CONFIG_SEND_AUTO_STOP
static int hi_mci_wait_auto_stop_complete(struct himci_host *host)
{
    unsigned int cmd_retry_count = 0;
    unsigned long time_out_;
    unsigned int cmd_irq_reg = 0;
    unsigned long flags = 0;

    mmc_trace(2, "begin");
    mmc_assert(host);

    time_out_ = mmc_get_sys_ticks() + request_timeout;
    while (1) {
        do {
            mmc_irq_lock(&flags);
            cmd_irq_reg = readl((unsigned int)host->base + MCI_RINTSTS);
            if (cmd_irq_reg & ACD_INT_STATUS) {
                himci_writel((ACD_INT_STATUS | RTO_INT_STATUS
                            | RCRC_INT_STATUS | RE_INT_STATUS),
                        host->base + MCI_RINTSTS);
                mmc_irq_unlock(flags);
                return 0;
            }
            mmc_irq_unlock(flags);
            cmd_retry_count++;
        } while (cmd_retry_count < retry_count);

        cmd_retry_count = 0;
        if (host->card_status == CARD_UNPLUGED) {
            return -EIO;
        }
        if (mmc_get_sys_ticks() >= time_out_) {
            mmc_trace(3, "wait auto stop complete is timeout!");
            return -ETIMEDOUT;
        }
        mmc_thread_sched();
    }
}
#endif

static int hi_mci_data_sync(struct mmc_data *data)
{
    unsigned int sg_phyaddr, sg_length;
    unsigned int i = 0;

    if (data->data_flags & MMC_DATA_READ) {
        for (i = 0; i < data->sg_len; i++) {
            sg_length = sg_dma_len(&data->sg[i]);
            sg_phyaddr = sg_dma_address(&data->sg[i]);
            mmc_dma_cache_inv((void*)sg_phyaddr, sg_length);
        }
    }
    return 0;
}

static int hi_mci_wait_data_complete(struct himci_host *host)
{
    struct mmc_data *data = NULL;
    unsigned int req_timeout = 0;
    //UINT64 now = mmc_get_sys_ticks();
    UINT32 tfv;

    mmc_trace(2, "begin");
    mmc_assert(host);
    data = host->data;
    mmc_assert(data);

    req_timeout = (host->is_tuning)?(HI_MCI_TUNINT_REQ_TIMEOUT):(request_timeout);
    mmc_delay_us(10);  /*wait for a moment until sdio complete interrupt comming*/

    tfv = mmc_event_wait(&host->himci_event, HIMCI_PEND_DTO_M, req_timeout);
    if (tfv == LOS_ERRNO_EVENT_READ_TIMEOUT) {
        data->err = -ETIMEDOUT;

        if (!host->is_tuning) {
        mmc_trace(6, "wait data request complete is timeout! 0x%08X",
                host->irq_status);
        }
        hi_mci_idma_stop(host);
        hi_mci_data_done(host, host->irq_status);

        return -1;
    }

    hi_mci_idma_stop(host);
    hi_mci_data_done(host, host->irq_status);

    return 0;
}


static void hi_mci_finish_request(struct himci_host *host, struct mmc_request *mrq)
{
    mmc_trace(2, "begin");
    mmc_assert(host);
    mmc_assert(mrq);

//    host->mrq = NULL;
//    host->cmd = NULL;
//    host->data = NULL;
    //LOS_SemPost(host->sem_id);

    if (mrq->done)
        mrq->done(mrq);
    //mmc_request_done(host->mmc, mrq);
}


static int hi_mci_setup_data(struct himci_host *host, struct mmc_data *data)
{
    unsigned int sg_phyaddr, sg_length;
    unsigned int i = 0;
    unsigned int max_des = 0, des_cnt = 0;
    struct himci_des *des;
    unsigned int dma_dir;
#ifdef LOSCFG_DRIVERS_MMC_SPEEDUP
    unsigned int *dma_paddr;
#endif
    mmc_trace(2, "begin");
    mmc_assert(host);
    mmc_assert(data);

    host->data = data;

    if (data->data_flags & MMC_DATA_READ)
        dma_dir = DMA_FROM_DEVICE;
    else
        dma_dir = DMA_TO_DEVICE;

    host->dma_sg = data->sg;
    host->dma_sg_num = data->sg_len;

    /* cache ops */
    mmc_assert(host->dma_sg_num);
    mmc_trace(2, "host->dma_sg_num is %d\n", host->dma_sg_num);
    mmc_trace(2, "host->dma_paddr is 0x%08X,host->dma_vaddr is 0x%08X\n",
            (unsigned int)host->dma_paddr,
            (unsigned int)host->dma_vaddr);
#ifndef LOSCFG_DRIVERS_MMC_SPEEDUP
    max_des = (HIMCI_PAGE_SIZE/sizeof(struct himci_des));
    des = (struct himci_des *)host->dma_vaddr;
#else
    max_des = (HIMCI_PAGE_SIZE/sizeof(struct himci_des) - 1);
    des = (struct himci_des *)(host->wr_cmd_des + 1);
    dma_paddr = (unsigned int *)(host->cmd_paddr + PAGE_SIZE * host->wr_pos +
            sizeof(struct himci_cmd_des));
#endif

    for (i = 0; i < host->dma_sg_num; i++) {
        sg_length = sg_dma_len(&data->sg[i]);
        sg_phyaddr = sg_dma_address(&data->sg[i]);
        mmc_trace(2, "sg[%d] sg_length is 0x%08X, sg_phyaddr is 0x%08X\n",
                i, (unsigned int)sg_length, (unsigned int)sg_phyaddr);
        if((sg_phyaddr & (CACHE_ALIGNED_SIZE - 1)) != 0) {
            mmc_printf(2, "sg_phyaddr:0x%x sg_length:0x%x\n",sg_phyaddr,sg_length);
            MMC_BUG_ON();
        }

        /* we need clean cache here if mmc_write, but mmc_read after dma transfer */
        if (DMA_TO_DEVICE == dma_dir)
            mmc_dma_cache_clean((void*)sg_phyaddr, sg_length);
        else
            mmc_dma_cache_inv((void*)sg_phyaddr, sg_length);

        while (sg_length) {
            des[des_cnt].idmac_des_ctrl = DMA_DES_OWN | DMA_DES_NEXT_DES;
            des[des_cnt].idmac_des_buf_addr = sg_phyaddr;
#ifndef LOSCFG_DRIVERS_MMC_SPEEDUP
            /* idmac_des_next_addr is paddr for dma */
            des[des_cnt].idmac_des_next_addr = host->dma_paddr
                + (des_cnt + 1) * sizeof(struct himci_des);
#else
            des[des_cnt].idmac_des_next_addr = (unsigned long)
                (dma_paddr + (des_cnt + 1) * 4);
#endif
            if (sg_length >= 0x1000) {
                des[des_cnt].idmac_des_buf_size = 0x1000;
                sg_length -= 0x1000;
                sg_phyaddr += 0x1000;
            } else {
                /* FIXME:data alignment */
                des[des_cnt].idmac_des_buf_size = sg_length;
                sg_length = 0;
            }

            des_cnt++;
        }
        mmc_assert(des_cnt <= max_des);
    }
    des[0].idmac_des_ctrl |= DMA_DES_FIRST_DES;
    des[des_cnt - 1].idmac_des_ctrl |= DMA_DES_LAST_DES;
    des[des_cnt - 1].idmac_des_next_addr = 0;

    /* sync dma descriptors buffer before dma starts */
#ifndef LOSCFG_DRIVERS_MMC_SPEEDUP
    mmc_dma_cache_clean(host->dma_vaddr, HIMCI_PAGE_SIZE);
#else
    mmc_dma_cache_clean((unsigned int *)(host->cmd_paddr + PAGE_SIZE * host->wr_pos), HIMCI_PAGE_SIZE);
#endif
    for (i=0; i<des_cnt; i++) {
        mmc_trace(2, "address of des[%d] is 0x%08X",
                i, (unsigned int)&des[i]);
        mmc_trace(2, "des[%d].idmac_des_ctrl is 0x%08X",
                i, (unsigned int)des[i].idmac_des_ctrl);
        mmc_trace(2, "des[%d].idmac_des_buf_size is 0x%08X",
                i, (unsigned int)des[i].idmac_des_buf_size);
        mmc_trace(2, "des[%d].idmac_des_buf_addr 0x%08X",
                i, (unsigned int)des[i].idmac_des_buf_addr);
        mmc_trace(2, "des[%d].idmac_des_next_addr is 0x%08X",
                i, (unsigned int)des[i].idmac_des_next_addr);

        }

    return 0;
}

static int hi_mci_wait_card_complete(struct himci_host *host,
        struct mmc_data *data)
{
    unsigned int card_retry_count = 0;
    unsigned int card_status_reg = 0;

    UINT64 time_out_;

    mmc_trace(2, "begin");
    mmc_assert(host);
    /* mmc_assert(data); */

    //time_out_ = LOS_TickCountGet() + request_timeout;
    time_out_ = mmc_get_sys_ticks() + request_timeout;
    while (1) {

        do {
            card_status_reg = readl((unsigned int)host->base + MCI_STATUS);
            if (!(card_status_reg & DATA_BUSY)) {
                mmc_trace(2, "end, card_retry_count:%d", card_retry_count);
                return 0;
            }
            card_retry_count++;
        } while (card_retry_count < retry_count);

        card_retry_count = 0;

        if (host->card_status == CARD_UNPLUGED) {
            host->mrq->cmd->err = -ETIMEDOUT;
            mmc_trace(5, "card is unpluged!");
            return -1;
        }
        if (mmc_get_sys_ticks() >= time_out_) {
            data->err = -ETIMEDOUT;
            mmc_trace(5, "wait card ready complete is timeout!");
            return -1;
        }
        mmc_thread_sched();
    }
}

void mmc_do_request(struct mmc_host *mmc, struct mmc_request *mrq)
{
    struct himci_host *host = NULL;
    int byte_cnt, tmp_reg, fifo_count = 0;
#ifdef CONFIG_SEND_AUTO_STOP
    int trans_cnt;
#endif
    int ret = 0;
    unsigned long flags = 0;
    /*
       if ((himci_list[host->id] != host)) {
       mmc_trace(5, "err:mmc:%x,host:%x", mmc, host);
       mmc_trace(5, "cmd: %d", mrq->cmd->cmd_code);
       }
       */
    mmc_assert(mmc);
    mmc_assert(mrq);
    host = mmc->priv;
    mmc_assert(host);

    (void)mmc_sem_pend(host->sem_id, MMC_SEM_WAIT_FOREVER);
#ifndef LOSCFG_DRIVERS_MMC_SPEEDUP
    host->mrq = mrq;
    host->irq_status = 0;

    if (host->card_status == CARD_UNPLUGED) {
        mrq->cmd->err = -ENODEV;
        goto  request_end;
    }

    ret = hi_mci_wait_card_complete(host, mrq->data);
    if (ret) {
        //mrq->cmd->err = ret;
        mmc_err("wait card complete fail!\n");
        goto request_end;
    }

    /* prepare data */
    if (mrq->data) {
        ret = hi_mci_setup_data(host, mrq->data);
        if (ret) {
            mrq->data->err = ret;
            mmc_err("data setup is error!\n");

            goto request_end;
        }

        byte_cnt = mrq->data->blocksz * mrq->data->blocks;

        himci_writel(byte_cnt, host->base + MCI_BYTCNT);
        himci_writel(mrq->data->blocksz, host->base + MCI_BLKSIZ);

        tmp_reg = himci_readl(host->base + MCI_CTRL);
        tmp_reg |= FIFO_RESET;
        himci_writel(tmp_reg, host->base + MCI_CTRL);

        do {
            tmp_reg = himci_readl(host->base + MCI_CTRL);
            fifo_count++;
            if (fifo_count >= retry_count) {
                mrq->data->err = -ETIMEDOUT;
                goto request_end;
            }
        } while (tmp_reg&FIFO_RESET);

        /* start DMA */
        hi_mci_idma_start(host);
    } else {
        himci_writel(0, host->base + MCI_BYTCNT);
        himci_writel(0, host->base + MCI_BLKSIZ);
    }

    mmc_event_clear(&host->himci_event, HIMCI_PEND_DTO_M);
    /* send command */
    ret = hi_mci_exec_cmd(host, mrq->cmd, mrq->data);
    if (ret) {
        mrq->cmd->err = ret;
        hi_mci_idma_stop(host);
        mmc_trace(5, "cmd execute is error!");
        goto request_end;
    }

    /* wait command send complete */
    (void)hi_mci_wait_cmd_complete(host);

    /* start data transfer */
    if (mrq->data) {
        if (!(mrq->cmd->err) || (host->is_tuning)) {
            mmc_irq_lock(&flags);
            tmp_reg = himci_readl(host->base + MCI_INTMASK);
            tmp_reg |= DATA_INT_MASK;
            himci_writel(tmp_reg, host->base + MCI_INTMASK);
            mmc_irq_unlock(flags);
            //FIXME: if mmc_read , we need invalidate cache before  dma operation.
            //hi_mci_data_sync(mrq->data);
            /* wait data transfer complete */
            (void)hi_mci_wait_data_complete(host);
            hi_mci_data_sync(mrq->data);
        } else {
            hi_mci_idma_stop(host);
        }

        //if (mrq->stop) {
        if (MMC_CMD_STOPENABLE(mrq->data)) {
#ifdef CONFIG_SEND_AUTO_STOP
            trans_cnt = himci_readl(host->base + MCI_TCBCNT);
            if ((trans_cnt == byte_cnt) && (!(host->is_tuning))) {
                mmc_trace(2, "byte_cnt = %d, trans_cnt = %d",
                        byte_cnt, trans_cnt);
                ret = hi_mci_wait_auto_stop_complete(host);
                if (ret) {
                    mmc_err("stop cmd:ret = %d\n",ret);
                    mrq->data->cmd_stop.err = -ETIMEDOUT;
                    goto request_end;
                }
            } else {
#endif
                /* send stop command */
                mmc_trace(2, "this time, send soft stop");
                ret = hi_mci_exec_cmd(host, &(host->mrq->data->cmd_stop),
                        host->data);
                if (ret) {
                    mmc_err("stop cmd:ret = %d\n",ret);
                    mrq->data->cmd_stop.err = ret;
                    goto request_end;
                }
                ret = hi_mci_wait_cmd_complete(host);
                if (ret)
                    goto request_end;
#ifdef CONFIG_SEND_AUTO_STOP
            }
#endif
        }
    }
#else /*LOSCFG_DRIVERS_MMC_SPEEDUP*/
    {
        struct himci_cmd_des    *cmd_des;
        unsigned int rd_pos;
        host->mrq = mrq;
        host->cmd = mrq->cmd;
        host->data = mrq->data;
        himci_prep_entire(host, mrq);
        cmd_des = host->wr_cmd_des;
        if (mrq->data) {
            ret = hi_mci_setup_data(host, mrq->data);
            if (ret) {
                mmc_trace(3, "data setup is error!");
                goto request_end;
            }
            byte_cnt = mrq->data->blocksz * mrq->data->blocks;
            cmd_des->blk_sz = mrq->data->blocksz;
            cmd_des->byte_cnt = byte_cnt;
            mmc_trace(3, "blk_sz:%d blk_cnt:%d",
                    mrq->data->blocksz, mrq->data->blocks);
        } else {
            cmd_des->blk_sz = 0;
            cmd_des->byte_cnt = 0;
        }
        ret = hi_mci_exec_cmd(host, mrq->cmd, mrq->data);

        mmc_dma_cache_clean(cmd_des,sizeof(struct himci_cmd_des));
        if (ret)
            goto request_end;
        if (host->card_status == CARD_UNPLUGED) {
            mrq->cmd->err = -ENODEV;
            if (host->cmd->resp_type & MMC_CMD_TYPE_RW)
                host->mmc->status = MMC_HOST_ERR;
            goto request_end;
        }


        host->wr_pos++;
        host->wr_pos %= ADMA_QUEUE_DEEPTH;
        himci_writel(host->wr_pos, host->base + ADMA_Q_WRPTR);
        if (host->cmd->cmd_code == SD_SWITCH_VOLTAGE) {
            ret = hi_mci_wait_cmd_complete(host);
            if (ret)
                mmc_trace(5, "voltage switch failed!");
            rd_pos = host->wr_pos ? host->wr_pos -1 : ADMA_QUEUE_DEEPTH - 1;
            host->pmrq[rd_pos] = NULL;
            goto request_end;
        }

        if (!(host->cmd->resp_type & MMC_CMD_TYPE_RW)) {
            unsigned int tfv;
            tfv = mmc_event_wait(&host->himci_event, HIMCI_INTR_WAIT_M,
                    HIMCI_INTR_TIMEOUT);
            if (tfv == LOS_ERRNO_EVENT_READ_TIMEOUT) {
                unsigned int rstat;
                unsigned int qstat;
                unsigned int cmd_err;
                unsigned int data_err;

                rstat = himci_readl(host->base + MCI_RINTSTS);
                qstat = himci_readl(host->base + MCI_IDSTS);

                himci_err_check(host, qstat, rstat,
                        &cmd_err, &data_err);
                mrq->cmd->err = -ETIMEDOUT;
                mmc_trace(3, "cmd timeout");
            }
            rd_pos = host->wr_pos ? host->wr_pos - 1
                : ADMA_QUEUE_DEEPTH - 1;
            host->pmrq[rd_pos] = NULL;
            himci_read_response(host, mrq->cmd);
            goto request_end;
        }
        (void)mmc_sem_post(host->sem_id);
        return ;
    }
#endif
request_end:
    /* clear MMC host intr */
    mmc_irq_lock(&flags);
    himci_writel(ALL_INT_CLR & (~(SDIO_INT_STATUS)), host->base + MCI_RINTSTS);
    mmc_irq_unlock(flags);

    host->mrq = NULL;
    host->cmd = NULL;
    host->data = NULL;
    (void)mmc_sem_post(host->sem_id);
    hi_mci_finish_request(host, mrq);
}

int mmc_voltage_switch(struct mmc_host *mmc, enum signal_volt signal_voltage)
{
    struct himci_host *host = mmc->priv;
    uint32_t ctrl;

    ctrl = himci_readl(host->base + MCI_UHS_REG);

    if (signal_voltage == SIGNAL_VOLT_3V3 && (g_sq_dev[host->id].detect_by_1V8 != DETECT_BY_1V8_ENABLE)) {

        ctrl &= ~(HI_SDXC_CTRL_VDD_180 << host->port);
        himci_writel(ctrl, host->base + MCI_UHS_REG);
        mmc_sleep_ms(10);
        ctrl = himci_readl(host->base + MCI_UHS_REG);
        if (!(ctrl & (HI_SDXC_CTRL_VDD_180 << host->port))) {
            hi_mci_pad_ctrl_cfg(host, SIGNAL_VOLT_3V3);
            return 0;
        } else {
            mmc_err(": Switching to 3.3V " \
                    "signalling voltage failed\n");
            return -EIO;
        }
    } else if ((!(ctrl & HI_SDXC_CTRL_VDD_180 << host->port) &&
            (signal_voltage == SIGNAL_VOLT_1V8)) || g_sq_dev[host->id].detect_by_1V8 == DETECT_BY_1V8_ENABLE) {
        mmc_delay_us(10);
        hi_mci_control_cclk(host, DISABLE);
        ctrl |= (HI_SDXC_CTRL_VDD_180 << host->port);
        himci_writel(ctrl, host->base + MCI_UHS_REG);
        mmc_sleep_ms(10);
        ctrl = himci_readl(host->base + MCI_UHS_REG);
        if (ctrl & (HI_SDXC_CTRL_VDD_180 << host->port)) {
            hi_mci_control_cclk(host, ENABLE);
            mmc_sleep_ms(10);
            if (host->mmc->caps2.bits.caps2_HS200_1v8_SDR || \
                    host->mmc->caps2.bits.caps2_HS200_1v2_SDR) {
                hi_mci_pad_ctrl_cfg(host, SIGNAL_VOLT_1V8);
                return 0;
            }

            if (g_sq_dev[host->id].detect_by_1V8 == DETECT_BY_1V8_ENABLE)
                return 0;
            ctrl = himci_readl(host->base + MCI_RINTSTS);
            if ((ctrl & VOLT_SWITCH_INT_STATUS)
                    && (ctrl & CD_INT_STATUS)) {

                writel(VOLT_SWITCH_INT_STATUS | CD_INT_STATUS,(unsigned int)host->base + MCI_RINTSTS);
                hi_mci_pad_ctrl_cfg(host, SIGNAL_VOLT_1V8);
                return 0;
            }
        }
        ctrl &= ~(HI_SDXC_CTRL_VDD_180 << host->port);
        himci_writel(ctrl, host->base + MCI_UHS_REG);

        mmc_sleep_ms(10);
        hi_mci_ctrl_power(host, HOST_POWER_OFF, FORCE_DISABLE);

        mmc_sleep_ms(10);
        hi_mci_ctrl_power(host, HOST_POWER_ON, FORCE_DISABLE);

        hi_mci_control_cclk(host, DISABLE);
        mmc_sleep_ms(1);
        hi_mci_control_cclk(host, ENABLE);

        mmc_err(": Switching to 1.8V signalling " \
                "voltage failed, retrying with S18R set to 0\n");
        return -EAGAIN;
    } else
        return 0;
}

#ifndef HIMCI_EDGE_TUNING
int mmc_execute_tuning(struct mmc_host *mmc, uint32_t cmd_code)
{
    struct  himci_host *host;
    unsigned long flags = 0;
    unsigned int index, count;
    unsigned int err = 0;
    unsigned int prev_err = PHASE_NOT_FOUND;
    unsigned int phase_scale = 8; /* The num of phase shift is 8 */
    unsigned int found = 0; /* identify if we have found a valid phase */
    unsigned int reg_value = 0;
    unsigned int start_point = TUNING_START_PHASE;
    unsigned int end_point = TUNING_END_PHASE;
    unsigned int raise_point = PHASE_NOT_FOUND;
    unsigned int fall_point = PHASE_NOT_FOUND;

    struct mmc_cmd cmd = {0};
    //struct mmc_cmd stop = {0};
    struct mmc_request mrq = {NULL};
    struct mmc_data data = {0};
    //uint32_t cmd_cmd_code = cmd_code;
    struct scatterlist sg;
    struct tuning_blk_info tuning_info = {NULL, 0};
    void *data_buf;

    host = mmc->priv;
    host->is_tuning = true; /* come into phase tuning process */
    himci_writel(ALL_INT_CLR, host->base + MCI_RINTSTS);

    switch (cmd_code) {
        case SEND_TUNING_BLOCK_HS200:
            if(mmc->card_list->card.iocfg.bus_width == BUS_WIDTH_8) {
                tuning_info.blk = (uint8_t *)tuning_blk_8bit;
                tuning_info.blk_size = sizeof(tuning_blk_8bit);
            } else if (mmc->card_list->card.iocfg.bus_width == BUS_WIDTH_4) {
                tuning_info.blk = (uint8_t *)tuning_blk_4bit;
                tuning_info.blk_size = sizeof(tuning_blk_4bit);
            }
            break;
        case SEND_TUNING_BLOCK:
            //mmc_trace(3, "command %d send", \
            SEND_TUNING_BLOCK);
            //cmd.flags = MMC_RESP_R1 | MMC_CMD_ADTC;
            tuning_info.blk = (uint8_t *)tuning_blk_4bit;
            tuning_info.blk_size = sizeof(tuning_blk_4bit);
            break;
        case SWITCH_FUNC:
            //mmc_trace(3, "command %d send", SWITCH_FUNC);
            tuning_info.blk = (uint8_t *)tuning_blk_4bit;
            tuning_info.blk_size = sizeof(tuning_blk_4bit);
            //cmd.resp_type = MMC_RESP_R1 | MMC_CMD_ADTC;
            break;
        default:
            mmc_trace(3, "command %d not support", cmd_code);
            return -EINVAL;
            //goto tuning_out;
    }

    data_buf = (void *)memalign(CACHE_ALIGNED_SIZE, ALIGN(tuning_info.blk_size,CACHE_ALIGNED_SIZE));
    if (data_buf == NULL)
        return -ENOMEM;
    memset(data_buf, 0, tuning_info.blk_size);
    sg_init_one(&sg, data_buf, tuning_info.blk_size);

    himci_writel(0x1, host->base + MCI_CARDTHRCTL);
    mmc_trace(3, "start sd3.0 phase tuning...");
    for (index = start_point; index <= end_point; index++) {
        cmd.cmd_code = cmd_code;
        cmd.arg = 0;
        cmd.retries = 0;
        //cmd.data = NULL;
        cmd.resp_type = MMC_RESP_R1 | MMC_CMD_ADTC;
        cmd.err = 0;
        data.err = 0;

        mrq.data = &data;
        mrq.data->blocksz = tuning_info.blk_size;
        mrq.data->blocks = 1;
        mrq.data->data_flags = MMC_DATA_READ;
        mrq.data->sg = &sg;
        mrq.data->sg_len = 1;
        /*
           mrq.stop = &stop;
           mrq.stop->cmd_code = STOP_TRANSMISSION;
           mrq.stop->arg = 0;
           mrq.stop->resp_type = MMC_RESP_R1B | MMC_CMD_AC;
           */
#ifndef LOSCFG_DRIVERS_MMC_SPEEDUP
        mrq.data->data_flags |= MMC_CMD_STOP;
        mrq.data->cmd_stop.cmd_code = STOP_TRANSMISSION;
        mrq.data->cmd_stop.arg = 0;
        mrq.data->cmd_stop.resp_type = MMC_RESP_R1B | MMC_CMD_AC;
#else
        mrq.data->data_flags &= ~(MMC_CMD_STOP);
#endif

        mrq.cmd = &cmd;
        host->mrq = &mrq;

        reg_value = himci_readl(host->base + MCI_UHS_EXT);
        reg_value &= ~CLK_SMPL_PHS_MASK;
        reg_value |= (index << CLK_SMPL_PHS_OFFSET);
        himci_writel(reg_value, host->base + MCI_UHS_EXT);
        himci_writel(ALL_INT_CLR, host->base + MCI_RINTSTS);

        count = 0;
        do {
            cmd.err = 0;
            data.err = 0;
            memset(data_buf, 0, tuning_info.blk_size);
            mmc_wait_for_req(mmc, &mrq);
#ifdef LOSCFG_DRIVERS_MMC_SPEEDUP
            himci_send_stop(mmc);
#endif
            if ((cmd.err) || (data.err)) {
                mmc_trace(3, "cmd.err=%d data.err=%d",
                        cmd.err, data.err);
                err = 1;
                break;
            } else {
                if (memcmp(tuning_info.blk, data_buf, tuning_info.blk_size))
                {
                    mmc_trace(3, "pattern blk not matched!");
                    err = 1;
                    break;
                }
            }
            count++;
        } while (count < 1);
        if (!err) {
            found = 1;
        }
        if (index > start_point) {
            if (err && !prev_err)
                fall_point = index - 1;
            if (!err && prev_err)
                raise_point = index;
        }

        if ((raise_point != PHASE_NOT_FOUND) && (fall_point != PHASE_NOT_FOUND))
            goto tuning_out;

        prev_err = err;
        err = 0;
    }

tuning_out:
    free(data_buf);
    host->is_tuning = 0; /* jump out phase tuning process */
    if (!found) {
        mmc_trace(5, "mmc %d: no valid phase shift! use default",
                host->id);
        himci_writel(PHASE_SHIFT, host->base + MCI_UHS_EXT);
    } else {
        mmc_trace(3, "Tuning finished!!");
        himci_get_phase(host, raise_point, fall_point);
    }
    himci_writel(READ_THRESHOLD_SIZE, host->base + MCI_CARDTHRCTL);
    himci_writel(ALL_INT_CLR, host->base + MCI_RINTSTS);
    return 0;
}
#else
static void himci_edge_tuning_enable(struct himci_host *host)
{
    unsigned int val = 0;

    val = himci_readl(host->base + MCI_TUNING_CTRL);
    val |= HW_TUNING_EN;
    himci_writel(val, host->base + MCI_TUNING_CTRL);
    val = himci_readl(host->base + MCI_TUNING_CTRL);
}
static void himci_edge_tuning_disable(struct himci_host *host)
{
    unsigned int val;
    val = himci_readl(host->base + MCI_TUNING_CTRL);
    val &= ~HW_TUNING_EN;
    himci_writel(val, host->base + MCI_TUNING_CTRL);
}
static void himci_set_sap_phase(struct himci_host *host, unsigned int phase)
{
    unsigned int val;
    unsigned int phase_a;

    phase_a = phase >= 2 ? phase - 2 : phase + 6;
    val = himci_readl(host->base + MCI_UHS_EXT);
    val &= ~(CLK_SMPL_PHS_MASK | CLK_SMPLA_PHS_MASK);
    val |= (phase << CLK_SMPL_PHS_OFFSET) | (phase_a << CLK_SMPLA_PHS_SHIFT);
    himci_writel(val, host->base + MCI_UHS_EXT);
}
int mmc_send_tuning(struct mmc_host *mmc, unsigned int opcode, int *cmd_error)
{
    struct mmc_request mrq = {NULL};
    struct mmc_cmd cmd = {0};
    struct mmc_data data = {0};
    struct scatterlist sg;
    struct tuning_blk_info tuning_info = {NULL, 0};
    void *data_buf;
    int  err = 0;
    if(mmc->card_list->card.iocfg.bus_width == BUS_WIDTH_8) {
        tuning_info.blk = (uint8_t *)tuning_blk_8bit;
        tuning_info.blk_size = sizeof(tuning_blk_8bit);
    } else if (mmc->card_list->card.iocfg.bus_width == BUS_WIDTH_4) {
        tuning_info.blk = (uint8_t *)tuning_blk_4bit;
        tuning_info.blk_size = sizeof(tuning_blk_4bit);
    } else
       return -EINVAL;
    data_buf = (void *)memalign(CACHE_ALIGNED_SIZE, ALIGN(tuning_info.blk_size,CACHE_ALIGNED_SIZE));
    if (data_buf == NULL)
        return -ENOMEM;
    memset(data_buf, 0, tuning_info.blk_size);
    sg_init_one(&sg, data_buf, tuning_info.blk_size);
    mrq.cmd = &cmd;
    mrq.data = &data;

    cmd.arg = 0;
    cmd.retries = 0;
    cmd.cmd_code = opcode;
    cmd.resp_type = MMC_RESP_R1 | MMC_CMD_ADTC;
    cmd.err = 0;
    data.err = 0;

    mrq.data->blocksz = tuning_info.blk_size;
    mrq.data->blocks = 1;
    mrq.data->data_flags = MMC_DATA_READ;
    mrq.data->sg = &sg;
    mrq.data->sg_len = 1;

    mrq.data->data_flags |= MMC_CMD_STOP;
    mrq.data->cmd_stop.cmd_code = STOP_TRANSMISSION;
    mrq.data->cmd_stop.arg = 0;
    mrq.data->cmd_stop.resp_type = MMC_RESP_R1B | MMC_CMD_AC;
    mmc_wait_for_req(mmc, &mrq);
    if (cmd_error) {
        *cmd_error = cmd.err;
    }
    if (cmd.err) {
        err = cmd.err;
        goto out;
    }
    if (data.err) {
        err = data.err;
        goto out;
    }
    if (memcmp(tuning_info.blk, data_buf, tuning_info.blk_size)) {
        err = -EIO;
    }
out:
    free(data_buf);
    return err;
}

int mmc_execute_tuning(struct mmc_host *mmc, uint32_t cmd_code)
{
    struct  himci_host *host;
    unsigned int index,val;
    unsigned int found = 0, prev_found = 0, prev_point = 0;
    unsigned int start_point = PHASE_NOT_FOUND;
    unsigned int end_point = PHASE_NOT_FOUND;
    unsigned int phase = 0;
    host = mmc->priv;
    himci_edge_tuning_enable(host);

    for (index = 0; index < HIMCI_PHASE_SCALE; index++) {
        himci_set_sap_phase(host, index);

        mmc_send_tuning(mmc, cmd_code, NULL);
        val = himci_readl(host->base + MCI_TUNING_CTRL);
        found = val & FOUND_EDGE;
        mmc_trace(3, "try phase:%02d, found:0x%x\n", index, found);
        if (prev_found && !found) {
            end_point = prev_point;
        } else if (!prev_found && found) {
            if (index != 0)
                start_point = index;
        }
        if ((start_point != PHASE_NOT_FOUND) && (end_point != PHASE_NOT_FOUND))
            goto scan_out;
        prev_point = index;
        prev_found = found;
        found = 0;
    }
scan_out:
    if ((start_point == PHASE_NOT_FOUND) && (end_point == PHASE_NOT_FOUND)) {
        mmc_trace(5, "no valid phase shift! use default");
        return 0;
    }

    if (start_point == PHASE_NOT_FOUND)
        start_point = end_point;

    if (end_point == PHASE_NOT_FOUND)
        end_point = start_point;

    mmc_trace(5,"tuning  found edge on (s:%d, e:%d)",
           start_point, end_point);

    if (start_point > end_point)
        end_point += HIMCI_PHASE_SCALE;

    phase = ((start_point + end_point) / 2) % HIMCI_PHASE_SCALE;

    phase += HIMCI_PHASE_SCALE / 2;
    phase %= HIMCI_PHASE_SCALE;

    himci_set_sap_phase(host, phase);

    himci_edge_tuning_disable(host);

    himci_writel(ALL_INT_CLR, host + MCI_RINTSTS);

    mmc_trace(5, "determing final phase %d\n", phase);

    return 0;
}
#endif

#define MAX_BLOCK_NUM  2048
#define MAX_BLOCK_SIZE  512
static int hi_mci_host_config(const int num)
{
    int ret = 0;
    mmc_thread task_id;
    struct mmc_host *mmc = NULL;
    struct himci_host *host = NULL;

    mmc = malloc(sizeof(struct mmc_host));
    if (!mmc) {
        mmc_err("no mem for mmc_host\n");
        goto err;
    }
    host = malloc(sizeof(struct himci_host));
    if (!host) {
        mmc_err("no mem for himci\n");
        goto err;
    }
    memset(mmc, 0, sizeof(struct mmc_host));
    memset(host, 0, sizeof(struct himci_host));
    host->mmc = mmc;
    mmc->priv = host;
    host->id = num;
    set_mmc_host(mmc, num);
    mmc_trace(2,"id: %d, host[0x%x],mmc:[0x%8x].\n", num, host, mmc);

    host->dma_vaddr = (unsigned int *)memalign(CACHE_ALIGNED_SIZE, HIMMC_PAGE_SIZE);
    if (!(host->dma_vaddr)) {
        mmc_err("no mem for himci dma!\n");
        goto err;
    }
    /* initialize host */
    host->dma_paddr = (dma_addr_t)(host->dma_vaddr);
    mmc_trace(MMC_NOTICE, "MMC:host[%d],dma paddr is:0x%8x.\n",
            host->id, host->dma_paddr);

	if (host->id == MMC1) 
		host->card_status = CARD_IGNORE;
	else
    	host->card_status = CARD_UNPLUGED;
    //host->pre_detect = HOST_NOT_PRE_DETECT;
    /* initialize mmc */
    mmc->idx = num;
    mmc->ocr_default.bits.vdd_3v2_3v3 = 1;
    mmc->ocr_default.bits.vdd_3v3_3v4 = 1;
    mmc->max_blk_num = MAX_BLOCK_NUM;
    mmc->max_blk_size = MAX_BLOCK_SIZE;
    mmc->max_request_size = (mmc->max_blk_num) * (mmc->max_blk_size);

    mmc->caps.bits.cap_4_bit = 1;
    mmc->caps.bits.cap_mmc_highspeed = 1;
    mmc->caps.bits.cap_sd_highspeed = 1;
    mmc->caps.bits.cap_sdio_irq = 1;
    mmc->caps.bits.cap_UHS_SDR12 = 1;
    mmc->caps.bits.cap_UHS_SDR25 = 1;
    mmc->caps.bits.cap_UHS_SDR50 = 1;
    mmc->caps.bits.cap_UHS_SDR104 = 1;

    if (g_sdioirq_disable)
        mmc->caps.bits.cap_sdio_irq = 0;
    ret = hi_mci_board_config(host);
    if (ret)
        goto err;
    if (mmc_event_init(&(host->himci_event))) {
        mmc_err("event init fail!");
        ret = -EACCES;
        goto err;
    }
    if (mmc_sem_create(&(host->sem_id))) {
        mmc_err("semaphore init fail!");
        ret = -EACCES;
        goto err;
    }
    if (mmc_mutex_init(&(host->thread_mutex))) {
        mmc_err("mutex init fail!");
        ret = -EACCES;
        goto err;
    }
    ret = request_irq(host->irq_num, (irq_handler_t)hisd_irq, 0, "MMC-SD", host);
    if (ret) {
        mmc_err("request irq for himci is err");
        ret = -EACCES;
        goto err;
    }
    /* FIXME: should not be here */
    hal_interrupt_unmask((int)host->irq_num);
    /* create a thread for pre detect card pluged or unpluged */
    ret = mmc_thread_create(5,
            hi_mci_pre_detect_card,
            HIMCI_STACKSIZE,
            (void *)host,
            "himci_Pre_detect",
            &task_id);
    if (ret) {
        mmc_trace(5,"himci_Pre_detect create fail");
        goto err;
    }
    return 0;

err:
    if (mmc)
        free(mmc);
    if (host) {
        if (host->dma_vaddr)
            free(host->dma_vaddr);
        free(host);
    }
    set_mmc_host(NULL, num);
    mmc_err("err while initialize MMC %d!", num);
    return ret;
}


/* initialize config and register, create task ,e.g.*/
static int hi_mci_initialize(void)
{
    static bool mmc_initialized = false;
    mmc_thread task_id;
    int ret = 0;
    int i = 0;
    struct mmc_host *mmc;
    struct himci_host *host;

    if (mmc_initialized) {
        mmc_trace(5, "warn: hi mci has been initialized!");
        return -EPERM;
    }
    mmc_initialized = true;
    /* alloc memory for mmc host point */
    ret = mmc_alloc_mmc_list();
    if (ret)
        return ret;
    /* block init for mmc block */
    ret = block_init();
    if (ret)
        goto err;

    himci_proc_init();
    /* *
     * allocate himci and mmc memory, then initialize them
     * some event, semaphore, irq, thread should be applied here.
     * */
    for(i=0; i<MAX_MMC_NUM; i++) {
        ret = USE_THIS_MMC(i);
        if (!ret)
            continue;
        ret = hi_mci_host_config(i);
        if (ret) {
            goto err;
        }
    }
    /* create a thread for detecting card pluged or unpluged */
    ret = mmc_thread_create(10,
            sd_mmc_thread,
            HIMCI_STACKSIZE,
            (void *)NULL,
            "himci_Task",
            &task_id);
    if (ret) {
        mmc_trace(5, "himci_Task create fail");
        goto err;
    }
    return 0;

err:
    mmc_err("!!!!err while initialize MMC !");
    MMC_BUG_ON();
    return -ENOMEM;
}

/* Host init entry */
int SD_MMC_Host_init(void)
{
#ifdef MMC_POWERUP_TIME_CAL
    extern unsigned long long hi_sched_clock(void);
    sd_time_s = hi_sched_clock();
#endif
    mmc_printf(5,"\n********mmc host init ! ********");
    return hi_mci_initialize();
}

/* Guarding thread */
static void sd_mmc_thread(UINT32 p1, UINT32 p2, UINT32 p3, UINT32 p4)
{
    struct mmc_host *mmc = NULL;
    int i = 0;
    do {
        mmc = get_mmc_host(i);
        /* if not removeable, not detect any more */
        if (mmc && (!(mmc->caps.bits.cap_nonremovable))) {
            hi_mci_detect_card((struct himci_host *)mmc->priv);
        }
        mmc_sleep_ms(300/MAX_MMC_NUM);
        if(++i >= MAX_MMC_NUM)
            i = 0;
    } while (1);
    return;
}

static inline void mmc_signal_sdio_irq(struct mmc_card *card)
{
    card->host->is_sdio_irq_pending = true;
    if (card->is_sdio_event_useable)
        mmc_event_signal(&card->sdio_event, 1);
}

static irqreturn_t hisd_irq(int irq, void *data)
{
#ifndef LOSCFG_DRIVERS_MMC_SPEEDUP
    struct himci_host *host = (struct himci_host *)data;
    unsigned long state = 0;
    unsigned int mask_state = 0;
    struct mmc_card *card = host->mmc->card_cur;

    state = himci_readl(host->base + MCI_RINTSTS);

    if(host->mmc->caps.bits.cap_sdio_irq) {
        if ((NULL != card) && (is_card_sdio(card))) {
            mask_state = himci_readl(host->base + MCI_INTMASK);
            if ((state & SDIO_INT_STATUS) && (mask_state & SDIO_INT_MASK)) {
                himci_writel(SDIO_INT_STATUS , host->base + MCI_RINTSTS);
                mask_state &= ~SDIO_INT_MASK;
                himci_writel(mask_state, host->base + MCI_INTMASK);
                mmc_signal_sdio_irq(card);
            }
        }
    }
    if (state & DATA_INT_MASK) {

        host->irq_status = himci_readl(host->base + MCI_RINTSTS);
        himci_writel(DATA_INT_MASK , host->base + MCI_RINTSTS);

        mask_state = himci_readl(host->base + MCI_INTMASK);
        mask_state &= ~DATA_INT_MASK;
        himci_writel(mask_state, host->base + MCI_INTMASK);
        mmc_event_signal(&host->himci_event, HIMCI_PEND_DTO_M);
    }
#else
    {
        struct himci_host *host = (struct himci_host *)data;
        struct mmc_card *card = host->mmc->card_cur;
        struct mmc_request *mrq;
        struct mmc_cmd *cmd;
        unsigned int rd_pos = 0;
        unsigned int rstat = 0;
        unsigned int qstat = 0;
        unsigned int mask_state = 0;
        int cmd_err = 0;
        int data_err = 0;
        rstat = himci_readl(host->base + MCI_RINTSTS);
        qstat = himci_readl(host->base + MCI_IDSTS); /* clear ADMA3 intr */
        himci_writel(qstat, host->base + MCI_IDSTS);
        mmc_trace(3, "queue state:0x%x raw state:0x%x", qstat, rstat);
        if (!(qstat & ADMA_INT_ALL) && !(rstat & SDIO_INT_STATUS)) {
            mmc_trace(3, "##### no irq, should never be here! ######");
            return IRQ_NONE;
        }
        if (host->mmc->caps.bits.cap_sdio_irq) {
            if ((NULL != card) && (is_card_sdio(card))) {
                mask_state = himci_readl(host->base + MCI_INTMASK);
                if ((rstat & SDIO_INT_STATUS) && (mask_state & SDIO_INT_MASK)) {
                    himci_writel(SDIO_INT_STATUS,host->base + MCI_RINTSTS);
                    mask_state &= ~SDIO_INT_MASK;
                    himci_writel(mask_state, host->base + MCI_INTMASK);
                    mmc_signal_sdio_irq(card);
                }
            }
        }
        rd_pos = himci_get_stable_rd_pos(host, qstat, rstat);
        himci_err_check(host, qstat, rstat, &cmd_err, &data_err);
        mrq = host->pmrq[rd_pos];
        host->pmrq[rd_pos] = NULL;
        if (!mrq || !mrq->cmd) {
            //mmc_trace(5, "##################");
            //mmc_trace(5, "queue state:0x%x raw state:0x%x", qstat, rstat);
            //mmc_trace(5, "current rd:%d", rd_pos);
            return IRQ_NONE;
        }
        if (cmd_err || data_err) {
            if (mrq->cmd->resp_type & MMC_CMD_TYPE_RW) {
                host->error_count++;
                host->mmc->status = MMC_HOST_ERR;
            }
            mrq->cmd->err = cmd_err;
            if (mrq->data)
                mrq->data->err = data_err;
        }
        if (mrq->cmd->resp_type & MMC_CMD_TYPE_RW) {
            himci_read_response(host, mrq->cmd);
            hi_mci_finish_request(host, mrq);
        } else {
            mmc_event_signal(&host->himci_event, HIMCI_INTR_WAIT_M);
        }
    }
#endif

    return IRQ_HANDLED;
}

#ifdef LOSCFG_DRIVERS_MMC_SPEEDUP

unsigned int himci_get_stable_rd_pos(struct himci_host *host,
        unsigned int qstat, unsigned int rstat)
{
    unsigned int rd_pos;
    unsigned int fsm_stat;
    volatile unsigned int new_pos;
    volatile unsigned int new_fsm;
    unsigned int count = 0;
    int try = 1000;

    rd_pos = himci_readl(host->base + ADMA_Q_RDPTR);
    fsm_stat = himci_readl(host->base + MCI_IDSTS);
    while (try > 0) {
        new_pos = himci_readl(host->base + ADMA_Q_RDPTR);
        new_fsm = himci_readl(host->base + MCI_IDSTS);
        if ((rd_pos == new_pos) && (fsm_stat == new_fsm)) {
            count++;
        } else {
            rd_pos = new_pos;
            fsm_stat = new_fsm;
            count = 0;
        }

        if (count == 3)
            break;
        try--;
    }

    /* when error occur, whether the read ptr jump next is depend on
     * host's state machine when CES && ADMA3_FSM=4 && FSM!=0,
     * rdptr don't jump next
     */
    if (!(qstat & PACKET_TO_INT)) {
        if (!((qstat & CES) && (((fsm_stat >> ADMA3_FSM) & 0xF) == 4)
                    && ((fsm_stat >> FSM) & 0xF)))
            rd_pos = rd_pos ? (rd_pos - 1) : (ADMA_QUEUE_DEEPTH - 1);
    }

    return rd_pos;
}

int himci_send_stop(struct mmc_host *host)
{
    struct mmc_cmd cmd = {0};
    int err;

    cmd.cmd_code = STOP_TRANSMISSION;
    cmd.resp_type = MMC_RESP_R1B | MMC_CMD_AC;
    cmd.arg = 0;
    cmd.resp_type &= ~(MMC_CMD_TYPE_RW);
    err = mmc_wait_for_cmd(host, &cmd, 0);

    return err;
}

void himci_prep_entire(struct himci_host *host,
        struct mmc_request *mrq)
{
    struct himci_entire_des *ent_des;
    struct himci_cmd_des    *cmd_des;
    unsigned int wr_pos;

    wr_pos = himci_readl(host->base + ADMA_Q_WRPTR);
    mrq->wr_pos = wr_pos; /* only for ADMA3 R/W */
    host->pmrq[wr_pos] = mrq;   /* save send mrq */
    host->wr_pos = wr_pos;

    mmc_trace(3, "write des to position %d", wr_pos);
    ent_des = (struct himci_entire_des *)(host->dma_vaddr);
    ent_des = ent_des + wr_pos;
    host->wr_ent_des = ent_des;

    ent_des->cmd_des_addr = host->cmd_paddr + PAGE_SIZE * wr_pos;
    if ((host->cmd->resp_type & MMC_CMD_NON_BLOCKING)
            || (mrq->cmd->cmd_code == SD_SWITCH_VOLTAGE)) {
        mmc_trace(2, "command is non-blocking");
        ent_des->ctrl = 0x80000000;
    } else {
        mmc_trace(2, "command is block");
        ent_des->ctrl = 0xa0000000;
    }

    //mmc_trace(3, "ent des addr: 0x%x", host->dma_vaddr
    //        + sizeof(struct himci_entire_des) * wr_pos);
    mmc_trace(3,"ent des addr: 0x%x", ent_des);
    mmc_trace(3, "ent des ctrl = 0x%x, cmd_addres = 0x%x\n",
            ent_des->ctrl, ent_des->cmd_des_addr);

    mmc_dma_cache_clean(ent_des, sizeof(struct himci_entire_des));
    cmd_des = (struct himci_cmd_des *)(host->cmd_paddr);
    cmd_des = cmd_des + (256 * wr_pos);
    host->wr_cmd_des = cmd_des;
}

#define CMD_ERRORS                          \
    (RSP_R1_OUT_OF_RANGE |  /* Command argument out of range */ \
     RSP_R1_ADDRESS_ERROR | /* Misaligned address */        \
     RSP_R1_BLOCK_LEN_ERROR |   /* Transferred block length incorrect */\
     RSP_R1_WP_VIOLATION |  /* Tried to write to protected block */ \
     RSP_R1_CC_ERROR |      /* Card controller error */     \
     RSP_R1_ERROR)      /* General/unknown error */



static void himci_read_response(struct himci_host *host,
        struct mmc_cmd* cmd)
{
    if (cmd->resp_type & RESP_PRESENT) {
        if (cmd->resp_type & RESP_136) {
            cmd->resp[3] = himci_readl(host->base + MCI_RESP0);
            cmd->resp[2] = himci_readl(host->base + MCI_RESP1);
            cmd->resp[1] = himci_readl(host->base + MCI_RESP2);
            cmd->resp[0] = himci_readl(host->base + MCI_RESP3);
        } else {
            cmd->resp[0] = himci_readl(host->base + MCI_RESP0);
            cmd->resp[1] = 0;
            cmd->resp[2] = 0;
            cmd->resp[3] = 0;
        }
    }
    if ((cmd->resp_type & MMC_CMD_TYPE_RW) && (cmd->resp[0] & CMD_ERRORS))
        host->error_count++;
    mmc_trace(3, "cmd->resp[0] = 0x%x\n",cmd->resp[0]);
}

void himci_reset_host(struct himci_host *host)
{
     unsigned int reg_value;

     /* DMA Controller resets */
     reg_value = himci_readl(host->base + ADMA_CTRL);
     reg_value &= ~(ADMA3_EN);
     himci_writel(reg_value, host->base + ADMA_CTRL);
     reg_value = himci_readl(host->base + MCI_BMOD);
     reg_value |= BMOD_SWR;
     himci_writel(reg_value, host->base + MCI_BMOD);
     himci_writel(0xFFFFFFFF, host->base + MCI_IDSTS);

     /* ADMA3: reset queue R/W ptr */
     reg_value = himci_readl(host->base + ADMA_CTRL);
     reg_value |= (RDPTR_MOD_EN);
     reg_value &= ~(ADMA3_EN);
     himci_writel(reg_value, host->base + ADMA_CTRL);
     himci_writel(0, host->base + ADMA_Q_WRPTR);
     himci_writel(0, host->base + ADMA_Q_RDPTR);

     himci_writel(0xFFFFFFFF, host->base + MCI_RINTSTS);
     mmc_delay_us(800);

     reg_value = himci_readl(host->base + ADMA_CTRL);
     reg_value &= ~RDPTR_MOD_EN;
     reg_value |= (ADMA3_EN);
     himci_writel(reg_value, host->base + ADMA_CTRL);

     /* ADMA3: restart */
     reg_value = himci_readl(host->base + ADMA_CTRL);
     reg_value |= ADMA3_RESTART;
     himci_writel(reg_value, host->base + ADMA_CTRL);

     /* dma enable */
     //himci_idma_start(host);
     reg_value = himci_readl(host->base + MCI_BMOD);
     reg_value |= BMOD_DMA_EN;
     himci_writel(reg_value, host->base + MCI_BMOD);
}

void himci_err_check(struct himci_host *host,
        unsigned int qstat, unsigned int rstat,
        unsigned int *cmd_err, unsigned int *data_err)
{

    *cmd_err = 0;
    *data_err = 0;

    if (!(qstat & (ADMA_INT_ERR | CES)))
        return;

    if (qstat & ADMA_INT_ERR) {
        mmc_trace(5, "ADMA err! qstat:%x rstat:%x\n", qstat, rstat);
        *cmd_err = -ETIMEDOUT;
    }

    if (qstat & CES) {
        if (rstat & RTO_INT_STATUS) {
            mmc_trace(3, "response timeout error!");
            *cmd_err = -ETIMEDOUT;
        } else if (rstat & (RCRC_INT_STATUS | RE_INT_STATUS)) {
            mmc_trace(3, "response crc error!");
            *cmd_err = -EILSEQ;
        }

        if (rstat & (HTO_INT_STATUS | DRTO_INT_STATUS)) {
            mmc_trace(3, "data transfer timeout error!");
            *data_err = -ETIMEDOUT;
        } else if (rstat & (EBE_INT_STATUS | SBE_INT_STATUS |
                    FRUN_INT_STATUS | DCRC_INT_STATUS)) {
            mmc_trace(3, "data transfer error!");
            *data_err = -EILSEQ;
        }
    }

    if (*cmd_err || *data_err)
        himci_reset_host(host);
}
#endif
/* end of file himci.c */
