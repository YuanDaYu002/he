#include "los_typedef.h"
#include "los_printf.h"

#include "pm_device.h"
#include "pm_cpu.h"
#include "pm_cpu_avs.h"

#include "hal_cpu_avs.h"
#include "pm_hal.h"

struct avs_device cpu_avs_dev;

static void cpu_update_cur_avs_info(void)
{
    int i;
    struct avs_device *avs_dev = &cpu_avs_dev;
    struct pm_domain_device *pm_domain_dev = avs_dev->pm_domain_dev;
    avs_dev->cur_freq = pm_domain_dev->cur_freq;
    /* get default hpm record */
    for (i = 0; i < avs_dev->hpm_opp_num; i++)
    {
        if (avs_dev->cur_freq== avs_dev->hpm_opp_table[i].freq)
        {
            avs_dev->cur_hpm      = avs_dev->hpm_opp_table[i].hpmrecord;
            avs_dev->cur_volt_min = avs_dev->hpm_opp_table[i].vmin;
            avs_dev->cur_volt_max = avs_dev->hpm_opp_table[i].vmax;
            avs_dev->div          = avs_dev->hpm_opp_table[i].div;

            /* cpu HPM freq */
            hal_cpu_set_hpm_div(avs_dev->div);
            break;
        }
    }
}

static void cpu_hpm_init(void)
{
    hal_cpu_hpm_init();
}

static int cpu_get_average_hpm(void)
{
    return hal_cpu_get_average_hpm();
}

static void cpu_set_volt_accord_hpm(int hpm_delta, unsigned int volt_min, unsigned int volt_max)
{
    struct pm_domain_device *pm_domain_dev = cpu_avs_dev.pm_domain_dev;
    struct regulator_device *regulator = pm_domain_dev->regulator_dev;
    //int flag=0;

    if (hpm_delta < CPU_AVS_HPM_DELTA_MIN)
    {
        /* up 10mv */
        if (pm_domain_dev->cur_volt + CPU_AVS_VOLT_STEP <= volt_max)
        {
            //flag = 1;
            regulator->set_voltage(regulator, pm_domain_dev->cur_volt + CPU_AVS_VOLT_STEP);
            pm_domain_dev->cur_volt = pm_domain_dev->cur_volt + CPU_AVS_VOLT_STEP;
        }
    }
    else if (hpm_delta >= CPU_AVS_HPM_DELTA_MAX)
    {
        /*down 10mv */
        if (pm_domain_dev->cur_volt - CPU_AVS_VOLT_STEP >= volt_min)
        {
            //flag = 1;
            regulator->set_voltage(regulator, pm_domain_dev->cur_volt - CPU_AVS_VOLT_STEP);
            pm_domain_dev->cur_volt = pm_domain_dev->cur_volt - CPU_AVS_VOLT_STEP;
        }

    }
    //if (flag) {
    //    dprintf("%s(%0d):hpm_delta=%0d,volt_min=%0d,volt_max=%0d,cur_freq=%0d,cur_volt=%0d\n",__func__,__LINE__,hpm_delta,volt_min,volt_max,pm_domain_dev->cur_freq,pm_domain_dev->cur_volt);
    //}
}

int cpu_domain_avs_handle(void)
{
    int cur_average_hpm = 0,delta = 0;
    struct avs_device *avs_dev = &cpu_avs_dev;

    cpu_update_cur_avs_info();

    cur_average_hpm = cpu_get_average_hpm();
    delta = cur_average_hpm - avs_dev->cur_hpm;

    cpu_set_volt_accord_hpm(delta, avs_dev->cur_volt_min, avs_dev->cur_volt_max);

    return 0;
}

int cpu_domain_avs_init(struct pm_domain_device *pm_domain_dev)
{
    cpu_avs_dev.hpm_opp_table       = cpu_hpm_opp_table;
    cpu_avs_dev.hpm_opp_num         = ARRAY_SIZE(cpu_hpm_opp_table);
    cpu_avs_dev.handle              = cpu_domain_avs_handle;

    cpu_avs_dev.pm_domain_dev       = pm_domain_dev;
    pm_domain_dev->avs_dev          = &cpu_avs_dev;

    cpu_hpm_init();

    return 0;
}

