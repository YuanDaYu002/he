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

#include "sys_pm.h"
#include <string.h>
#include <linux/types.h>

#include "avs_pm.h"
#include "cpu.h"
#include "hi_io.h"
#include "pmc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define AVS_STEP 10 /*mv*/
#define AVS_INTERVAL 20 /*ms*/


static struct avs_dev cpudev={0};
extern int g_cur_profile;
/* hpm table define */
static HPM_VF_S cpu_freq_hpm_table[] =
{
    { HI_VDD_CPU_OPP0_FREQ, HI_VDD_CPU_OPP0_AVS_VMIN_MV, HI_VDD_CPU_OPP0_AVS_HPM, HI_VDD_CPU_OPP0_AVS_HPM_DIV},
    { HI_VDD_CPU_OPP1_FREQ, HI_VDD_CPU_OPP1_AVS_VMIN_MV, HI_VDD_CPU_OPP1_AVS_HPM, HI_VDD_CPU_OPP1_AVS_HPM_DIV},
    { HI_VDD_CPU_OPP2_FREQ, HI_VDD_CPU_OPP2_AVS_VMIN_MV, HI_VDD_CPU_OPP2_AVS_HPM, HI_VDD_CPU_OPP2_AVS_HPM_DIV},
    { HI_VDD_CPU_OPP3_FREQ, HI_VDD_CPU_OPP3_AVS_VMIN_MV, HI_VDD_CPU_OPP3_AVS_HPM, HI_VDD_CPU_OPP3_AVS_HPM_DIV},
};

/* after change the rate, reinitialize the hpm */
static int cpu_update_hpm(struct avs_dev *pstavs_dev)
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
            iSetPERI_PMC14cpu_hpm_div(pstavs_dev->div);    /*lint !e534*/
            break;
        }
    }

    return 0;
}

void cpu_AvsPause(void){
}

void cpu_AvsResume(void){
    struct avs_dev *pstavs_dev = &cpudev;
    pstavs_dev->cur_freq = cpu_freq_table[g_cur_profile].freq;
    pstavs_dev->cur_volt = cpu_freq_table[g_cur_profile].Volt;
    cpu_update_hpm(pstavs_dev);  /*lint !e534*/
    PRINTK("avs_cpu handler! cur_volt=%d, avs_dev->cur_freq=%d\n", pstavs_dev->cur_volt, pstavs_dev->cur_freq);
}

int avs_cpu_handler(void){
    struct avs_dev *pstavs_dev = &cpudev;
    unsigned long u32HpmCode, u32HpmCodeAverage = 0;
    unsigned long u32RegVal;
    int s32HpmDelta;

    /* read current code */
    u32HpmCodeAverage = 0;

    HI_RegRead(&u32RegVal, PERI_PMC_15);

    u32HpmCode = (u32RegVal & HPM_PC_RECORED_MASK);
    u32HpmCodeAverage += u32HpmCode;

    u32HpmCode = ((u32RegVal >> 12) & HPM_PC_RECORED_MASK);
    u32HpmCodeAverage += u32HpmCode;

    HI_RegRead(&u32RegVal, PERI_PMC_16);
    u32HpmCode = (u32RegVal & HPM_PC_RECORED_MASK);

    u32HpmCodeAverage += u32HpmCode;
    u32HpmCode = ((u32RegVal >> 12) & HPM_PC_RECORED_MASK);
    u32HpmCodeAverage += u32HpmCode;

    u32HpmCodeAverage = u32HpmCodeAverage / 4;

    s32HpmDelta = u32HpmCodeAverage - pstavs_dev->cur_hpm;
    /* compare code value */
    if (s32HpmDelta < 0x1)
    {
        /* up 10mv */

        if (pstavs_dev->cur_volt +  AVS_STEP <= pstavs_dev->cur_volt_max)
        {
            cpu_VoltScale(pstavs_dev->cur_volt +  AVS_STEP); /*lint !e534*/
            pstavs_dev->cur_volt = pstavs_dev->cur_volt +  AVS_STEP;
        }
    }
    else if (s32HpmDelta >= 0xa)
    {
        /*down 10mv */
        if (pstavs_dev->cur_volt - AVS_STEP >= pstavs_dev->cur_volt_min)
        {
            cpu_VoltScale(pstavs_dev->cur_volt -  AVS_STEP); /*lint !e534*/
            pstavs_dev->cur_volt = pstavs_dev->cur_volt -  AVS_STEP;
        }

    }

    return 0;
}

void cpu_HpmInit(struct avs_dev *pstavs_dev)
{
    /* set cpu avs control source */
    iSetPERI_PMC65cpu_avs_ctrl_sel(0x1);  /*lint !e534*/
    /* cpu HPM reset */
    iSetPERI_PMC14cpu_hpm_srst_req(0x1);  /*lint !e534*/
    iSetPERI_PMC14cpu_hpm_srst_req(0x0);  /*lint !e534*/
    /* cpu HPM limit*/
    iSetPERI_PMC17cpu_hpm_uplimit(pstavs_dev->hpm_uplimit);  /*lint !e534*/
    iSetPERI_PMC17cpu_hpm_lowlimit(pstavs_dev->hpm_downlimit);  /*lint !e534*/
    iSetPERI_PMC17cpu_hpm_monitor_period(pstavs_dev->high_period);  /*lint !e534*/
    iSetPERI_PMC30cpu_hpm_monitor_period(pstavs_dev->low_period);  /*lint !e534*/

    iSetPERI_PMC14cpu_hpm_shift(pstavs_dev->shift);  /*lint !e534*/
    iSetPERI_PMC14cpu_hpm_offset(pstavs_dev->offset);  /*lint !e534*/

    iSetPERI_PMC14cpu_hpm_monitor_en(0x1);  /*lint !e534*/
    iSetPERI_PMC14cpu_hpm_en(0x1);  /*lint !e534*/
}

int cpu_AvsInit(void){
    struct avs_dev *pstavs_dev = &cpudev;

    memcpy(pstavs_dev->freq_hpm_table, cpu_freq_hpm_table, sizeof(cpu_freq_hpm_table));
    pstavs_dev->max_hpm_tale_index = ARRAY_SIZE(cpu_freq_hpm_table);
    pstavs_dev->cur_freq = cpu_freq_table[g_cur_profile].freq;
    pstavs_dev->cur_volt_max = CPU_VMAX;
    pstavs_dev->cur_volt = cpu_freq_table[g_cur_profile].Volt;
    cpu_update_hpm(pstavs_dev);   /*lint !e534*/
    pstavs_dev->avs_enable = true;

    /* avs hpm */
    pstavs_dev->hpm_uplimit   = 276;
    pstavs_dev->hpm_downlimit = 190;
    pstavs_dev->high_period   = 0x1;
    pstavs_dev->low_period    = 0;
    pstavs_dev->shift         = 0;
    pstavs_dev->offset        = 0;

    pstavs_dev->hpm_pwm_dec_step   = 0;
    pstavs_dev->hpm_pwm_inc_step   = 0;
    pstavs_dev->hpm_fsm_mode       = 0;

    cpu_HpmInit(pstavs_dev);
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
