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
#include "hi_io.h"
#include "los_swtmr.h"
#include "pmc.h"
#include "media.h"
#include "los_cpup.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern int cpu_load(void);
extern void cpu_AvsResume(void);
extern int avs_cpu_handler(void);
extern int cpu_AvsInit(void);

static void dvfs_handler(unsigned int);

struct PM_DC_DC_ATTR_S g_stDcdcAttr = {0};
struct cpu_freq{
    int freq;
    int Volt;
}cpu_freq_table[4]={
    {400, 1020},
    {600, 1100},
    {732, 1200},
    {850, 1300},
};
static int uphold = 80;
static int downhold = 30;
int g_cur_profile = 0;
#define MAX_PROFILE 2
#define MIN_PROFILE 0

static int get_cpu_profile(void){
    unsigned long value;
    unsigned cpu_sel;
    unsigned int clk_rate;
    int i;
    HI_RegRead(&value, PERI_CRG12);
    cpu_sel = value & 0x300;
    if(0x100 == cpu_sel)
    {
        clk_rate = 500;
    }
    else if(0x200 == cpu_sel)
    {
        clk_rate = 400;
    }
    else
    {
        unsigned long cpu_sel_addr0, cpu_sel_addr1;
        unsigned fbdiv, frac, refdiv, pstdiv1, pstdiv2;
        if(0x0 == cpu_sel)
        {
            cpu_sel_addr0 = PERI_CRG0;
            cpu_sel_addr1 = PERI_CRG1;
        }
        else
        {
            cpu_sel_addr0 = PERI_CRG2;
            cpu_sel_addr1 = PERI_CRG3;
        }
        HI_RegRead(&value, cpu_sel_addr0);
        frac    = 0xFFFFFF & value;
        pstdiv1 = ((0x7 << 24) & value) >> 24;
        pstdiv2 = ((0x7 << 28) & value) >> 28;
        HI_RegRead(&value, cpu_sel_addr1);
        fbdiv   = 0xFFF & value;
        refdiv  = ((0x3F << 12) & value) >> 12;
        clk_rate = 24 * (fbdiv + (frac >> 24)) / (refdiv * pstdiv1 * pstdiv2);
    }
    for(i=0;i<4;i++){
        if(clk_rate == cpu_freq_table[i].freq)
            break;
    }
    if(i != 3)
        return i;
    else
        PRINTK("cpu_profile_init failed!\n");
    return -1;
}
static unsigned int get_cpu_load(void){
    return LOS_SysCpuUsage() / LOS_CPUP_PRECISION_MULT;
}

static int hi_get_cpu_duty(int uvolt)
{
    int duty, Vpwm;

    Vpwm = g_stDcdcAttr.Vref - ((uvolt * g_stDcdcAttr.VoltConfig - g_stDcdcAttr.Vout_base) / g_stDcdcAttr.Rconfig);
    duty = Vpwm * 100 / 3300 ;

    return duty;
}

int cpu_VoltScale(int uvolt)
{
    /*set duty */
    int duty;
    int freqN, freqM;
    duty = hi_get_cpu_duty(uvolt);
    freqN = g_stDcdcAttr.PwmFreqN;
    freqM = (duty * (freqN + 1)) / 100  - 1;

    iSetPERI_PMC1cpu_pwm_period(freqN);  /*lint !e534*/
    iSetPERI_PMC1cpu_pwm_duty(freqM);  /*lint !e534*/
    return 0;
}

static void HI_SetCpuApll(unsigned rate)
{
    unsigned int u32Frac, u32Postdiv1, u32Postdiv2, u32Fbdiv, u32Refdiv;

    switch (rate)
    {
        case (400):
        {
            u32Frac = 0;
            u32Postdiv1 = 1;
            u32Postdiv2 = 1;
            u32Refdiv = 24;
            u32Fbdiv = 400;
            break;
        }
        case (600):
        {
            u32Frac = 0;
            u32Postdiv1 = 2;
            u32Postdiv2 = 1;
            u32Refdiv = 1;
            u32Fbdiv = 50;
            break;
        }
        case (732):
        {
            u32Frac = 0;
            u32Postdiv1 = 1;
            u32Postdiv2 = 1;
            u32Refdiv = 24;
            u32Fbdiv = 732;
            break;
        }
        case (850):
        {
            u32Frac = 0;
            u32Postdiv1 = 1;
            u32Postdiv2 = 1;
            u32Refdiv = 24;
            u32Fbdiv = 850;
            break;
        }
        default:
            return;
    }
    HI_RegSetBitEx(u32Frac, 0, 24, PERI_CRG0);
    HI_RegSetBitEx(u32Postdiv1, 24, 3, PERI_CRG0);
    HI_RegSetBitEx(u32Postdiv2, 28, 3, PERI_CRG0);
    HI_RegSetBitEx(u32Fbdiv, 0, 12, PERI_CRG1);
    HI_RegSetBitEx(u32Refdiv, 12, 6, PERI_CRG1);

    return;
}
static int clk_set_rate(int freq)
{

    HI_RegSetBitEx(0x1, 8, 2, PERI_CRG12);
    HI_SetCpuApll(freq);

    /* 3.wait for the success of APLL LOCK */
    while (1)
    {
        unsigned long value;
        HI_RegRead(&value, PERI_CRG58);
        if (value & 0x1)
            break;
    }
    HI_RegSetBitEx(0x0, 8, 2, PERI_CRG12);
    return 0;
}

static int cpu_upprofile(int cur_profile){
    int freq, volt;
    freq = cpu_freq_table[cur_profile+1].freq;
    volt = cpu_freq_table[cur_profile+1].Volt;
    cpu_VoltScale(volt);    /*lint !e534*/
    clk_set_rate(freq);      /*lint !e534*/
    return 0;
}

static int cpu_downprofile(int cur_profile){
    int freq, volt;
    freq = cpu_freq_table[cur_profile-1].freq;
    volt = cpu_freq_table[cur_profile-1].Volt;
    PRINTK("down profile! freq=%d, volt=%d\n", freq, volt);
    clk_set_rate(freq);   /*lint !e534*/
    cpu_VoltScale(volt);  /*lint !e534*/
    return 0;
}

//static int flag = 0;
//static int cnt = 0;
static void dvfs_handler(unsigned int v){
    int swcpu_load = 0;
    swcpu_load = get_cpu_load();

    if(swcpu_load > uphold){
        if(g_cur_profile  < MAX_PROFILE){
            cpu_upprofile(g_cur_profile);  /*lint !e534*/
            g_cur_profile = get_cpu_profile();
            cpu_AvsResume();
            return;
        }
    }else if(swcpu_load < downhold){
        if(g_cur_profile > MIN_PROFILE){
            cpu_downprofile(g_cur_profile);  /*lint !e534*/
            g_cur_profile = get_cpu_profile();
            cpu_AvsResume();
            return;
        }
    }
    avs_cpu_handler();  /*lint !e534*/
    media_Handler();  /*lint !e534*/
}



static int dvfs_timer_init(void){
    UINT32 uwRet = 0;
    UINT16 usSwTmrID = 0;
    uwRet = LOS_SwtmrCreate(10, LOS_SWTMR_MODE_PERIOD, dvfs_handler, &usSwTmrID, 0);
    if(0 != uwRet){
        PRINTK("LOS_SwtmrCreate dvfs_handler err.\n");
        return -1;
    }
    uwRet = LOS_SwtmrStart(usSwTmrID);
    if(0 != uwRet){
        PRINTK("LOS_SwtmrStart  err.\n");
        return -1;
    }

    PRINTK("dvfs start timer!\n");
    return 0;
}

static int dvfs_enable(void){
    /* set power control way to be DC_DC(pwm) */
    iSetPERI_PMC65cpu_avs_ctrl_mux(0); /*lint !e534*/
    /* 1.set pin mux*/
    HI_RegSetBitEx(0x0, 0, 2, PERI_MUX86);

    /* 2.set pwm3 to control cpu  Power domain */
    iSetPERI_PMC4pwm3_reuse_cfg(0x1); /*lint !e534*/

    /* 4｡｢enable pwm*/
    iSetPERI_PMC4cpu_pwm_enable(1);  /*lint !e534*/

    iSetPERI_PMC67mda_avs_ctrl_mux(0);  /*lint !e534*/
    HI_RegSetBitEx(0x0, 0, 2, PERI_MUX85);

    iSetPERI_PMC4pwm2_reuse_cfg(0x3);  /*lint !e534*/

    iSetPERI_PMC4mda_pwm_enable(1);  /*lint !e534*/

    return 0;
}

static int pm_DCDCAttr_init(struct PM_DC_DC_ATTR_S *pstDcDcAttr)
{
    int ret = 0;

    pstDcDcAttr->Vref = PM_VREF;
    pstDcDcAttr->R1 = PM_R1;
    pstDcDcAttr->R2 = PM_R2;
    pstDcDcAttr->R3 = PM_R3 + PM_R4;
    pstDcDcAttr->R4 = PM_R5;

    pstDcDcAttr->Vout_base = pstDcDcAttr->Vref * (pstDcDcAttr->R1 + pstDcDcAttr->R2) * (pstDcDcAttr->R3 + pstDcDcAttr->R4);
    pstDcDcAttr->Rconfig = pstDcDcAttr->R1 * pstDcDcAttr->R2;
    pstDcDcAttr->VoltConfig = (pstDcDcAttr->R3 + pstDcDcAttr->R4) * pstDcDcAttr->R2;

    /* pwm's period is 120kHz,so the config N of pwm's period is changeless*/
    pstDcDcAttr->PwmFreqN = 199;
    return ret;
}


int dvfs_init(void){
    g_cur_profile = get_cpu_profile();
    pm_DCDCAttr_init(&g_stDcdcAttr);  /*lint !e534*/
    dvfs_enable();    /*lint !e534*/
    cpu_AvsInit();   /*lint !e534*/
    media_AvsInit();   /*lint !e534*/
    dvfs_timer_init();   /*lint !e534*/
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
