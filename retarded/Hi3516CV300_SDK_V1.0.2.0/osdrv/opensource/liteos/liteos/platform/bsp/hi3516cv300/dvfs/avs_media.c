/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/


#include "avs_pm.h"
#include "media.h"
#include "sys_pm.h"
#include "hi_io.h"
#include "string.h"
#include "pmc.h"
#define AVS_STEP 10 /*mv*/
#define AVS_INTERVAL 20 /*ms*/
extern struct PM_DC_DC_ATTR_S g_stDcdcAttr;
static struct avs_dev mediadev={0};
/* hpm table define */
static HPM_VF_S media_freq_hpm_table[] =
{
    { HI_VDD_MEDIA_OPP1_PROFILE, HI_VDD_MEDIA_OPP1_AVS_VMIN_MV, HI_VDD_MEDIA_OPP1_AVS_HPM, HI_VDD_MEDIA_OPP1_AVS_HPM_DIV},
    { HI_VDD_MEDIA_OPP2_PROFILE, HI_VDD_MEDIA_OPP2_AVS_VMIN_MV, HI_VDD_MEDIA_OPP2_AVS_HPM, HI_VDD_MEDIA_OPP2_AVS_HPM_DIV},
    { HI_VDD_MEDIA_OPP3_PROFILE, HI_VDD_MEDIA_OPP3_AVS_VMIN_MV, HI_VDD_MEDIA_OPP3_AVS_HPM, HI_VDD_MEDIA_OPP3_AVS_HPM_DIV},
    { HI_VDD_MEDIA_OPP4_PROFILE, HI_VDD_MEDIA_OPP4_AVS_VMIN_MV, HI_VDD_MEDIA_OPP4_AVS_HPM, HI_VDD_MEDIA_OPP4_AVS_HPM_DIV},
};

void media_AvsPause(void){
}

/* after change the rate, reinitialize the hpm */
static int media_update_hpm(struct avs_dev *pstavs_dev)
{
    unsigned int i;

    /* get default hpm record */
    for (i = 0; i < pstavs_dev->max_hpm_tale_index; i++)
    {
        if (pstavs_dev->cur_freq == pstavs_dev->freq_hpm_table[i].freq)
        {
            pstavs_dev->cur_hpm      = pstavs_dev->freq_hpm_table[i].hpmrecord;
            pstavs_dev->cur_volt_min = pstavs_dev->freq_hpm_table[i].vmin;

            pstavs_dev->div          = pstavs_dev->freq_hpm_table[i].div;
            /* cpu HPM freq */
            iSetPERI_PMC22mda_top_hpm_div(pstavs_dev->div); /*lint !e534*/
            break;
        }
    }

    return 0;
}

void media_AvsResume(void){
   struct avs_dev *pstavs_dev = &mediadev;
   /* this can get from register later*/
   pstavs_dev->cur_freq   = HI_VDD_MEDIA_OPP4_PROFILE;
   pstavs_dev->avs_enable = true;
   pstavs_dev->cur_volt = HI_VDD_MEDIA_OPP4_MV;
   media_update_hpm(pstavs_dev);  /*lint !e534*/
}

static void media_clk_init(void)
{
    int hw_profile;
    /* this can get from register later */
    /* get hw proflie */
    hw_profile = DEFAULT_HW_PROFILE;
    /* set sw profile */
    HI_RegSetBitEx(hw_profile, 16, 4, PROFILE_REG);

    /* set media sw available*/
    HI_RegSetBitEx(1, 20, 1, PROFILE_REG);
    return;
}

int hi_get_media_duty(int uvolt)
{
    int duty, Vpwm;

    Vpwm = g_stDcdcAttr.Vref - ((uvolt * g_stDcdcAttr.VoltConfig - g_stDcdcAttr.Vout_base) / g_stDcdcAttr.Rconfig);
    duty = Vpwm * 100 / 3300 ;

    return duty;
}

void media_VoltScale(unsigned int uvolt){
    int duty;
    int freqN, freqM;
    duty = hi_get_media_duty(uvolt);
    freqN = g_stDcdcAttr.PwmFreqN;
    freqM = (duty * (freqN + 1)) / 100  - 1;

    iSetPERI_PMC3mda_pwm_period(freqN);   /*lint !e534*/
    iSetPERI_PMC3mda_pwm_duty(freqM);   /*lint !e534*/
}

int media_Handler(void){
    struct avs_dev *pstavs_dev = &mediadev;
    unsigned long u32HpmCode, u32HpmCodeAverage = 0;
    unsigned long u32RegVal;
    int s32HpmDelta;
    /* read current code */
    u32HpmCodeAverage = 0;

    HI_RegRead(&u32RegVal, PERI_PMC_23);

    u32HpmCode = (u32RegVal & HPM_PC_RECORED_MASK);
    u32HpmCodeAverage += u32HpmCode;

    u32HpmCode = ((u32RegVal >> 12) & HPM_PC_RECORED_MASK);
    u32HpmCodeAverage += u32HpmCode;

    HI_RegRead(&u32RegVal, PERI_PMC_24);
    u32HpmCode = (u32RegVal & HPM_PC_RECORED_MASK);

    u32HpmCodeAverage += u32HpmCode;
    u32HpmCode = ((u32RegVal >> 12) & HPM_PC_RECORED_MASK);
    u32HpmCodeAverage += u32HpmCode;

    u32HpmCodeAverage = u32HpmCodeAverage / 4;

    s32HpmDelta = u32HpmCodeAverage - pstavs_dev->cur_hpm;
    /* compare code value */
    if (s32HpmDelta <= 0x1)
    {
        /* up 10mv */
        if (pstavs_dev->cur_volt < pstavs_dev->cur_volt_max)
        {
            media_VoltScale(pstavs_dev->cur_volt + AVS_STEP);
            pstavs_dev->cur_volt = pstavs_dev->cur_volt + AVS_STEP;
        }
    }
    else if (s32HpmDelta >= 0x10)
    {
        /*down 10mv */
        if (pstavs_dev->cur_volt > pstavs_dev->cur_volt_min)
        {
            media_VoltScale(pstavs_dev->cur_volt - AVS_STEP);
            pstavs_dev->cur_volt = pstavs_dev->cur_volt - AVS_STEP;
        }
    }

    return 0;
}

void media_HpmInit(struct avs_dev *pstavs_dev)
{
    /* hpm mode */
    iSetPERI_PMC67mda_avs_ctrl_sel(0x1);   /*lint !e534*/

    /* reset and then unreset */
    iSetPERI_PMC22mda_top_hpm_srst_req(0x1);   /*lint !e534*/
    iSetPERI_PMC22mda_top_hpm_srst_req(0x0);   /*lint !e534*/

    /* hpm ctrl*/
    iSetPERI_PMC25mda_top_hpm_uplimit(pstavs_dev->hpm_uplimit);   /*lint !e534*/
    iSetPERI_PMC25mda_top_hpm_lowlimit(pstavs_dev->hpm_downlimit);   /*lint !e534*/
    iSetPERI_PMC25mda_top_hpm_monitor_period(pstavs_dev->high_period);   /*lint !e534*/
    iSetPERI_PMC30mda_top_hpm_monitor_period(pstavs_dev->low_period);   /*lint !e534*/

    iSetPERI_PMC22mda_top_hpm_shift(pstavs_dev->shift);    /*lint !e534*/
    iSetPERI_PMC22mda_top_hpm_offset(pstavs_dev->offset);   /*lint !e534*/

    iSetPERI_PMC22mda_top_hpm_monitor_en(0x1);  /*lint !e534*/
    iSetPERI_PMC22mda_top_hpm_en(0x1);  /*lint !e534*/
}

int media_AvsInit(void){
    struct avs_dev *pstavs_dev = &mediadev;
    media_clk_init();
    memcpy(pstavs_dev->freq_hpm_table, media_freq_hpm_table, sizeof(media_freq_hpm_table));
    pstavs_dev->max_hpm_tale_index = ARRAY_SIZE(media_freq_hpm_table);
    pstavs_dev->cur_freq = HI_VDD_MEDIA_OPP4_PROFILE;
    pstavs_dev->cur_volt_max = MEDIA_VMAX;
    pstavs_dev->cur_volt = HI_VDD_MEDIA_OPP4_MV;
    media_update_hpm(pstavs_dev);  /*lint !e534*/

    pstavs_dev->avs_enable = true;
    pstavs_dev->hpm_uplimit   = 0x20;
    pstavs_dev->hpm_downlimit = 0x10;
    pstavs_dev->high_period   = 0x1;
    pstavs_dev->low_period    = 0;
    pstavs_dev->div           = 0xa;
    pstavs_dev->shift         = 0;
    pstavs_dev->offset        = 0;

    pstavs_dev->hpm_pwm_dec_step   = 0;
    pstavs_dev->hpm_pwm_inc_step   = 0;
    pstavs_dev->hpm_fsm_mode       = 0;
    media_HpmInit(pstavs_dev);


    return 0;
}

