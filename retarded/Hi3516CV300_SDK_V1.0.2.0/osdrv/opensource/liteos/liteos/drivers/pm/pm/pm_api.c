#include "los_cpup.h"
#include "los_swtmr.h"
#include "los_base.h"
#include "los_mux.h"

#include "pm_device.h"

#include "pm_regulator.h"
#include "pm_dvfs.h"
#include "pm_avs.h"
#include "pm_cpu_freq.h"
#include "pm_media_avs.h"

#include "pm_cpu.h"
#include "pm_media.h"

extern struct pm_device pm_dev;

/*
 *  dvfs_ctrl:cpu domain dvfs ctrl interface
 *  parameter:0:disable dvfs;  others:enable dvfs
 *  return:-1:fail;   0:ok
 */
int dvfs_ctrl(int enable)
{
    if (pm_dev.domain_num == 0) {
        dprintf("Error: no domain register!\n");
        return -1;
    }

    if (LOS_OK !=LOS_MuxPend(pm_dev.cpu_domain_device->freq_dev->lock, LOS_WAIT_FOREVER))
        return -1;

    if(enable)
        pm_dev.cpu_domain_device->freq_dev->dvfs_enable = 1;
    else
        pm_dev.cpu_domain_device->freq_dev->dvfs_enable = 0;

    if (LOS_OK !=LOS_MuxPost(pm_dev.cpu_domain_device->freq_dev->lock))
        return -1;

    return 0;
}

/*
 *  pm_ctrl:power manager ctrl interface
 *  parameter:0:disable pm;  others:enable pm
 *  return:-1:fail;   0:ok
 */
int pm_ctrl(int enable)
{
    if (pm_dev.domain_num == 0) {
        dprintf("Error: no domain register!\n");
        return -1;
    }

    if (LOS_OK !=LOS_MuxPend(pm_dev.lock, LOS_WAIT_FOREVER))
        return -1;

    if(enable)
        pm_dev.pm_enable = 1;
    else
        pm_dev.pm_enable = 0;

    if (LOS_OK !=LOS_MuxPost(pm_dev.lock))
        return -1;

    return 0;
}

/*
 * get_cpu_max_profile: get cpu domain max profile index
 * return:max profile index value
 */
int get_cpu_max_profile()
{
    if (pm_dev.domain_num == 0) {
        dprintf("Error: no domain register!\n");
        return -1;
    }

    return pm_dev.cpu_domain_device->freq_dev->profile_num - 1;
}

/*
 * get_cpu_profile: get cpu domain profile index
 * return:profile index value
 */
int get_cpu_profile()
{
    if (pm_dev.domain_num == 0) {
        dprintf("Error: no domain register!\n");
        return -1;
    }

    return cpu_domain_getprofile();
}

/*
 * set_cpu_profile: set cpu domain profile index
 * parameter:profile index
 * return:0:ok;   -1:fail
 */
int set_cpu_profile(int profile)
{
    if (pm_dev.domain_num == 0) {
        dprintf("Error: no domain register!\n");
        return -1;
    }

    int max_profile = pm_dev.cpu_domain_device->freq_dev->profile_num-1;

    if (pm_dev.pm_enable) {
        dprintf("ERROR:please disable pm!\n");
        return -1;
    }

    if (profile<0 || profile>max_profile) {
        dprintf("ERROR: set_profile=%0d must be:[0~%0d]\n",profile, max_profile);
        return -1;
    }

    cpu_domain_setprofile(profile);

    return 0;
}

/*
 * get_media_max_profile: get media domain max profile index
 * return:max profile index value
 */
int get_media_max_profile()
{
    if (pm_dev.domain_num == 0) {
        dprintf("Error: no domain register!\n");
        return -1;
    }

    return 3;
}

/*
 * get_media_profile: get media domain profile index
 * return:profile index value
 */
int get_media_profile()
{
    if (pm_dev.domain_num == 0) {
        dprintf("Error: no domain register!\n");
        return -1;
    }

    int profile = media_domain_getprofile();

    switch(profile) {
#ifdef HI3559
    case 0:
        profile = 2;
        break;
    case 1:
        profile = 3;
        break;
#endif
#ifdef HI3556
    case 0:
        profile = 1;
        break;
    case 1:
        profile = 2;
        break;
    case 2:
        profile = 3;
        break;
#endif
    default:
        profile = -1;
        dprintf("ERROR: profile[%0d] is invalid!\n",profile);
    }
    return profile;
}

/*
 * set_media_profile: set media domain profile index
 * parameter:profile index
 * return:0:ok;   -1:fail
 */
int set_media_profile(int profile)
{
    if (pm_dev.domain_num == 0) {
        dprintf("Error: no domain register!\n");
        return -1;
    }

    if (pm_dev.pm_enable) {
        dprintf("ERROR:please disable pm!\n");
        return -1;
    }

    switch(profile) {
#ifdef HI3559
    case 2:
        profile = 0;
        break;
    case 3:
        profile = 1;
        break;
#endif
#ifdef HI3556
    case 1:
        profile = 0;
        break;
    case 2:
        profile = 1;
        break;
    case 3:
        profile = 2;
        break;
#endif
    default:
        dprintf("ERROR: profile[%0d] is invalid!\n",profile);
        return -1;
    }

    media_domain_setprofile(profile);

    return 0;
}

/*
 * set_media_voltage: set media domain voltage
 * parameter:set_volt(uV)
 * return:0:ok;   -1:fail
 */
int set_media_voltage(int set_volt)
{
    if (pm_dev.domain_num == 0) {
        dprintf("Error: no domain register!\n");
        return -1;
    }

    struct regulator_device *regulator = pm_dev.cpu_domain_device->regulator_dev;

    if (pm_dev.pm_enable) {
        dprintf("ERROR:please disable pm!\n");
        return -1;
    }

    if (media_domain_setvoltage(set_volt))
        return -1;

    return 0;
}

