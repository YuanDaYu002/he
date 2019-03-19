#include "los_typedef.h"
#include "linux/kernel.h"

#include "pm_media.h"
#include "pm_media_avs.h"
#include "hal_media_avs.h"
#include "pm_hal.h"

struct avs_device media_avs_dev;

static int media_get_average_temperature(void)
{
    return hal_media_get_average_temperature();
}

static int media_get_profile(void)
{
    return hal_media_get_profile();
}

static void media_update_cur_avs_info(void)
{
    int i;
    int temperature = 0;
    int temperature_index = 0;
    struct avs_device *avs_dev = &media_avs_dev;

    /*Get current temperature and profile*/
    if(avs_dev->temp_num > 1)
    {
        temperature = media_get_average_temperature();

        for (i = 0; i < avs_dev->temp_num; i++)
        {
            if(temperature > avs_dev->hpm_opp_table[avs_dev->cur_profile*avs_dev->profile_num + i].temp)
            {
                temperature_index++;
            }
            else
            {
                break;
            }
        }

    }

    avs_dev->cur_freq =  avs_dev->cur_profile*avs_dev->profile_num + temperature_index;

    for (i = 0; i < avs_dev->hpm_opp_num; i++)
    {
        if (avs_dev->cur_freq  == avs_dev->hpm_opp_table[i].freq)
        {
            avs_dev->cur_hpm      = avs_dev->hpm_opp_table[i].hpmrecord;
            avs_dev->cur_volt_min = avs_dev->hpm_opp_table[i].vmin;
            avs_dev->cur_volt_max = avs_dev->hpm_opp_table[i].vmax;
            avs_dev->div          = avs_dev->hpm_opp_table[i].div;
            /* media HPM freq */
            hal_media_set_hpm_div(avs_dev->div);
            break;
        }
    }
}

static void media_hpm_init(void)
{
    hal_media_hpm_init();
}

static void media_tsensor_init(void)
{
    hal_media_tsensor_init();
}

static int media_get_average_hpm(void)
{
    return hal_media_get_average_hpm();
}

static void media_set_volt_accord_hpm(int hpm_delta, unsigned int volt_min, unsigned int volt_max)
{
    struct pm_domain_device *pm_domain_dev = media_avs_dev.pm_domain_dev;
    struct regulator_device *regulator = pm_domain_dev->regulator_dev;

    if(pm_domain_dev->cur_volt > volt_max)
    {
        pm_domain_dev->cur_volt -= MEDIA_AVS_VOLT_STEP;
        regulator->set_voltage(regulator, pm_domain_dev->cur_volt);
    }
    else if (pm_domain_dev->cur_volt < volt_min)
    {
        pm_domain_dev->cur_volt += MEDIA_AVS_VOLT_STEP;
        regulator->set_voltage(regulator, pm_domain_dev->cur_volt);
    }
    else if (hpm_delta < MEDIA_AVS_HPM_DELTA_MIN)
    {
        if (pm_domain_dev->cur_volt +  MEDIA_AVS_VOLT_STEP <= volt_max)
        {
            pm_domain_dev->cur_volt += MEDIA_AVS_VOLT_STEP;
            regulator->set_voltage(regulator, pm_domain_dev->cur_volt);
        }
    }
    else if (hpm_delta >= MEDIA_AVS_HPM_DELTA_MAX)
    {
        if (pm_domain_dev->cur_volt - MEDIA_AVS_VOLT_STEP >= volt_min)
        {
            pm_domain_dev->cur_volt -= MEDIA_AVS_VOLT_STEP;
            regulator->set_voltage(regulator, pm_domain_dev->cur_volt);
        }
    }
}

int media_domain_avs_handle(void)
{
    int cur_average_hpm = 0,delta = 0;
    struct avs_device *avs_dev = &media_avs_dev;
    struct pm_domain_device *pm_domain_dev = media_avs_dev.pm_domain_dev;
    struct regulator_device *regulator = pm_domain_dev->regulator_dev;

    pm_domain_dev->cur_volt = regulator->get_voltage(regulator);

    media_update_cur_avs_info();

    cur_average_hpm = media_get_average_hpm();
    delta = cur_average_hpm - avs_dev->cur_hpm;

    media_set_volt_accord_hpm(delta, avs_dev->cur_volt_min, avs_dev->cur_volt_max);
    //dprintf("cur_average_hpm=%0d,avs_dev->cur_hpm=%0d,avs_dev->cur_volt_min=%0d,avs_dev->cur_volt_max=%0d,pm_domain_dev->cur_volt=%0d\n",cur_average_hpm,avs_dev->cur_hpm,avs_dev->cur_volt_min,avs_dev->cur_volt_max,pm_domain_dev->cur_volt);
    return 0;
}

int media_domain_avs_init(struct pm_domain_device *pm_domain_dev)
{
    struct avs_device *avs_dev = &media_avs_dev;
    struct regulator_device *regulator = pm_domain_dev->regulator_dev;

    pm_domain_dev->avs_dev  = avs_dev;
    pm_domain_dev->cur_volt = regulator->get_voltage(regulator);

    avs_dev->pm_domain_dev  = pm_domain_dev;
    avs_dev->hpm_opp_table  = media_hpm_opp_table;
    avs_dev->hpm_opp_num    = ARRAY_SIZE(media_hpm_opp_table);
    avs_dev->profile_num    = media_prof_num;
#ifdef HI3559
    avs_dev->cur_profile    = 1;
#endif
#ifdef HI3556
    avs_dev->cur_profile    = 0;
#endif
    avs_dev->temp_num       = media_temp_num;
    avs_dev->handle         = media_domain_avs_handle;

    media_hpm_init();

    media_tsensor_init();

    return 0;
}

int media_domain_getprofile(void)
{
    return media_avs_dev.cur_profile;
}

void media_domain_setprofile(int profile)
{
    media_avs_dev.cur_profile = profile;
}

int media_domain_setvoltage(int set_volt)
{
    struct pm_domain_device *pm_domain_dev = media_avs_dev.pm_domain_dev;
    struct regulator_device *regulator = pm_domain_dev->regulator_dev;

    if (regulator->set_voltage(regulator, set_volt))
        return -1;

    return 0;
}

