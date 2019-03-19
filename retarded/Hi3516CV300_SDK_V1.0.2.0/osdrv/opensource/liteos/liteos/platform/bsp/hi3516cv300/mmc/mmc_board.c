#include "hisoc/mmc.h"

unsigned int hi_mci_clk_div(struct himci_host *host, unsigned int cclk)
{
    unsigned int clk_div = 0;
    unsigned int tmp_reg = 0;
    unsigned int host_clk = 0;

    if (host->id == 0) {
        tmp_reg = himci_readl(PERI_CRG49);
        tmp_reg &= ~(SDIO0_CLK_SEL_MASK);
        tmp_reg |= SDIO0_CLK_SEL_49_5M;
        host_clk = MMC_FREQ_49_5M;
        if (cclk < MMC_FREQ_49_5M) {
        clk_div = host_clk / (cclk * 2);
        if (host_clk % (cclk * 2))
            clk_div++;
        if (clk_div > 0xFF)
            clk_div = 0xFF;
        }
        himci_writel(tmp_reg, PERI_CRG49);
    } else if (host->id == 1) {
        tmp_reg = himci_readl(PERI_CRG49);
        tmp_reg &= ~(SDIO1_CLK_SEL_MASK);
        tmp_reg |= SDIO1_CLK_SEL_49_5M;
        host_clk = MMC_FREQ_49_5M;
        if (cclk < MMC_FREQ_49_5M) {
        clk_div = host_clk / (cclk * 2);
        if (host_clk % (cclk * 2))
            clk_div++;
        if (clk_div > 0xFF)
            clk_div = 0xFF;
        }
        himci_writel(tmp_reg, PERI_CRG49);
    } else if (host->id == 2) {
        tmp_reg = himci_readl(PERI_CRG49);
        tmp_reg &= ~(SDIO2_CLK_SEL_MASK);
        if (cclk >= MMC_FREQ_99M) {
            host_clk = MMC_FREQ_99M;
            tmp_reg |= SDIO2_CLK_SEL_99M;
        } else if (cclk >= MMC_FREQ_49_5M) {
            host_clk = MMC_FREQ_49_5M;
            tmp_reg |= SDIO2_CLK_SEL_49_5M;
        } else {
            tmp_reg |= SDIO2_CLK_SEL_49_5M;
            host_clk = MMC_FREQ_49_5M;
            clk_div = host_clk / (cclk * 2);
            if (host_clk % (cclk * 2))
                clk_div++;
            if (clk_div > 0xFF)
                clk_div = 0xFF;
        }
        himci_writel(tmp_reg, PERI_CRG49);
    } else if  (host->id == 3) {
        tmp_reg = himci_readl(PERI_CRG50);
        tmp_reg &= ~(SDIO3_CLK_SEL_MASK);
        tmp_reg |= SDIO3_CLK_SEL_49_5M;
        host_clk = MMC_FREQ_49_5M;
        if (cclk < MMC_FREQ_49_5M) {
            clk_div = host_clk / (cclk * 2);
            if (host_clk % (cclk * 2))
                    clk_div++;
            if (clk_div > 0xFF)
                    clk_div = 0xFF;
        }
        himci_writel(tmp_reg, PERI_CRG50);
    }
    host->hclk = host_clk;

    mmc_trace(3, "host_clk = %d, cclk = %d, clk_div = %d,crg49 = 0x%x\n",
            host_clk, cclk, clk_div, tmp_reg); /*lint !e506 */
    return clk_div;
}

void hi_mci_clock_cfg(struct himci_host *host)
{
    unsigned int reg_value = 0;


    switch (host->id) {
        case 0:
            reg_value = himci_readl(PERI_CRG49);
            reg_value &= ~(SDIO0_CLK_SEL_MASK);
            /* 16cv300 sdio0 only 49.5M */
            reg_value |= SDIO0_CLK_SEL_49_5M;
            reg_value |= SDIO0_CKEN;
            himci_writel(reg_value, PERI_CRG49);
            break;
        case 1:
            reg_value = himci_readl(PERI_CRG49);
            reg_value &= ~(SDIO1_CLK_SEL_MASK);
            /* 16cv300 sdio1 only 49.5M */
            reg_value |= SDIO1_CLK_SEL_49_5M;
            reg_value |= SDIO1_CKEN;
            himci_writel(reg_value, PERI_CRG49);
            break;
        case 2:
            reg_value = himci_readl(PERI_CRG49);
            reg_value &= ~(SDIO2_CLK_SEL_MASK);
            if (MMC_FREQ_99M == CONFIG_MMC2_CLK) /*lint !e506 */
                reg_value |= SDIO2_CLK_SEL_99M;
            else if (MMC_FREQ_49_5M == CONFIG_MMC2_CLK)
                reg_value |= SDIO2_CLK_SEL_49_5M;
            reg_value |= SDIO2_CKEN;
            himci_writel(reg_value, PERI_CRG49);
            break;
        case 3:
            reg_value = himci_readl(PERI_CRG50);
            reg_value &= ~(SDIO3_CLK_SEL_MASK);
            /* 16cv300 sdio3 only 49.5M */
            reg_value |= SDIO3_CLK_SEL_49_5M;
            reg_value |= SDIO3_CKEN;
            himci_writel(reg_value, PERI_CRG50);
            break;
        default:
            mmc_err("err:host->id = %d\n",(host->id));
            break;
    }
}

void hi_mci_pad_ctrl_cfg(struct himci_host *host, enum signal_volt voltage)
{
    if (host->id == 2) {
        if (SIGNAL_VOLT_1V8 == voltage) {
            himci_writel(EMMC_CLK_DS_1V8, REG_CTRL_EMMC_CCLK);
            himci_writel(EMMC_CMD_DS_1V8, REG_CTRL_EMMC_CCMD);
            himci_writel(EMMC_DATA0_DS_1V8, REG_CTRL_EMMC_CDATA0);
            himci_writel(EMMC_DATA1_DS_1V8, REG_CTRL_EMMC_CDATA1);
            himci_writel(EMMC_DATA2_DS_1V8, REG_CTRL_EMMC_CDATA2);
            himci_writel(EMMC_DATA3_DS_1V8, REG_CTRL_EMMC_CDATA3);
        }
    }
}

int hi_mci_board_config(struct himci_host *host)
{
    struct mmc_host * mmc = host->mmc;
    /* MMC0/1/3 only support SD 2.0*/
    if (host->id == MMC0) {
        host->base = (void *)SDIO0_REG_BASE;
        host->irq_num = NUM_HAL_INTERRUPT_SDIO0;
        mmc->freq_min = CONFIG_MMC0_CCLK_MIN;
        mmc->freq_max = CONFIG_MMC0_CCLK_MAX;
        mmc->caps.bits.cap_UHS_SDR12 = 0;
        mmc->caps.bits.cap_UHS_SDR25 = 0;
        mmc->caps.bits.cap_UHS_SDR50 = 0;
        mmc->caps.bits.cap_UHS_SDR104 = 0;
    } else if (host->id == MMC1) {
        host->base = (void *)SDIO1_REG_BASE;
        host->irq_num = NUM_HAL_INTERRUPT_SDIO1;
        mmc->freq_min = CONFIG_MMC1_CCLK_MIN;
        mmc->freq_max = CONFIG_MMC1_CCLK_MAX;
        mmc->caps.bits.cap_UHS_SDR12 = 0;
        mmc->caps.bits.cap_UHS_SDR25 = 0;
        mmc->caps.bits.cap_UHS_SDR50 = 0;
        mmc->caps.bits.cap_UHS_SDR104 = 0;
    } else if (host->id == MMC2) {
        mmc->caps.bits.cap_nonremovable = 1;
        //mmc->caps.bits.cap_8_bit = 1;
#ifdef EMMC_DDR50
        mmc->caps.bits.cap_UHS_DDR50 = 1;
#endif
#ifdef EMMC_HS200
        //mmc->caps.bits.cap_max_current_200 = 1;
        mmc->caps2.bits.caps2_HS200_1v8_SDR = 1;
        mmc->caps2.bits.caps2_HS200_1v2_SDR = 1;
#endif

        host->base = (void *)SDIO2_REG_BASE;
        host->irq_num = NUM_HAL_INTERRUPT_EMMC;
        mmc->freq_min = CONFIG_MMC2_CCLK_MIN;
        mmc->freq_max = CONFIG_MMC2_CCLK_MAX;
    } else if (host->id == MMC3) {
        host->port = 0;
        host->base = (void *)SDIO3_REG_BASE;
        host->irq_num = NUM_HAL_INTERRUPT_SDIO3;
        mmc->freq_min = CONFIG_MMC3_CCLK_MIN;
        mmc->freq_max = CONFIG_MMC3_CCLK_MAX;
        mmc->caps.bits.cap_UHS_SDR12 = 0;
        mmc->caps.bits.cap_UHS_SDR25 = 0;
        mmc->caps.bits.cap_UHS_SDR50 = 0;
        mmc->caps.bits.cap_UHS_SDR104 = 0;
    }
    else {
        mmc_err("err:host->id = %d\n",(host->id));
    }
    return 0;
}

int get_emmc_host_id(void)
{
    return MMC2;
}

void himci_cfg_phase(struct mmc_host *mmc, enum mmc_bus_timing timing)
{
    struct himci_host *host = mmc->priv;
    unsigned int reg_value = 0;

    if (timing == TIMING_UHS_DDR52) {
        reg_value = himci_readl(host->base + MCI_UHS_EXT); /*lint !e665*/
        reg_value &= ~CLK_SMPL_PHS_MASK;
        reg_value |= ((DDR50_DRV_PHASE_CFG << CLK_DRV_PHS_OFFSET));
        himci_writel(reg_value, host->base + MCI_UHS_EXT); /*lint !e665*/
    } else if (timing == TIMING_MMC_HS200) {
        reg_value = himci_readl(host->base + MCI_UHS_EXT); /*lint !e665*/
        reg_value &= ~CLK_DRV_PHS_MASK;
        reg_value |= ((SDR104_DRV_PHASE_CFG << CLK_DRV_PHS_OFFSET));
        himci_writel(reg_value, host->base + MCI_UHS_EXT); /*lint !e665*/
    }
    mmc_trace(3, "timing = %d,MCI_UHS_EXT = 0x%x", timing, reg_value); /*lint !e506 */
}
/* end of file board.c */
