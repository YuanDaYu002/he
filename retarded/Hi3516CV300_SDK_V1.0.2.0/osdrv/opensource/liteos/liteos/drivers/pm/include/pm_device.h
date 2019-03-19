#ifndef  __PM_DEVICE_H__
#define  __PM_DEVICE_H__

#include "los_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

struct pm_profile;
struct freq_device;
struct hpm_opp;
struct avs_device;
struct regulator_device;
struct pm_device;
struct pm_domain_device;

typedef int  domain_regulator_init(struct pm_domain_device *);
typedef int  domain_freq_init(struct pm_domain_device *);
typedef int  domain_avs_init(struct pm_domain_device *);

struct pm_profile
{
    int freq;   /* unit:Hz */
    int volt;   /* unit:uV */
    struct freq_device *freq_dev;
};

struct freq_device
{
    struct pm_domain_device *pm_domain_dev;

    struct pm_profile *pm_profile_table;

    int profile_num;

    int cur_profile;
    int cur_freq;

    int max_freq;
    int min_freq;

    int (*get_freq_clk)(void);
    int (*set_freq_clk)(int freq);
    int (*handle)(void);

    UINT32 lock;
    int dvfs_enable;
};

struct hpm_opp
{
    struct avs_device *avs_dev;

    int freq;          /* unit: kHz */
    int vmin;          /* unit: uV */
    int vmax;          /* unit: uV */
    int hpmrecord;     /* hpm record */
    int div;           /* frequency dividing ratio */
    int temp;          /* temperature */
    int profile;
};

struct avs_device
{
    struct pm_domain_device *pm_domain_dev;

    struct hpm_opp *hpm_opp_table;
    int hpm_opp_num;

    int cur_hpm;
    int cur_freq;
    int cur_volt_min;
    int cur_volt_max;
    int div;

    int temp_num;
    int temp;
    int profile_num;
    int cur_profile;

    int (*handle)(void);
};

struct regulator_device
{
    struct pm_domain_device *pm_domain_dev;

    int step_uV;
    int min_uV;
    int max_uV;

    int (*set_voltage) (struct regulator_device* regulator_dev, int set_volt);
    int (*get_voltage) (struct regulator_device* regulator_dev);
};

struct pm_domain_device
{
    struct pm_device *pm_dev;

    /* freq */
    int cur_freq; /* unit:Hz */
    struct freq_device *freq_dev;
    domain_freq_init *domain_freq_init;

    /* regulator_dev */
    int cur_volt; /* unit:uV */
    struct regulator_device *regulator_dev;
    domain_regulator_init *domain_regulator_init;

    /* avs  */
    struct avs_device *avs_dev;
    domain_avs_init *domain_avs_init;
};

struct pm_device
{
    int domain_num;

    UINT32 lock;
    int pm_enable;

    struct pm_domain_device *cpu_domain_device;
    struct pm_domain_device *media_domain_device;
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#undef _1K
#define _1K 1000ULL
#undef _1M
#define _1M 1000000ULL
#undef _1G
#define _1G 1000000000ULL

/*
 *  dvfs_ctrl:cpu domain dvfs ctrl interface
 *  parameter:0:disable dvfs;  others:enable dvfs
 *  return:-1:fail;   0:ok
 */
int dvfs_ctrl(int enable);

/*
 *  pm_ctrl:power manager ctrl interface
 *  parameter:0:disable pm;  others:enable pm
 *  return:-1:fail;   0:ok
 */
int pm_ctrl(int enable);

/*
 * get_cpu_max_profile: get cpu domain max profile index
 * return:max profile index value
 */
int get_cpu_max_profile(void);

/*
 * get_cpu_profile: get cpu domain profile index
 * return:profile index value
 */
int get_cpu_profile(void);

/*
 * set_cpu_profile: set cpu domain profile index
 * parameter:profile index
 * return:0:ok;   -1:fail
 */
int set_cpu_profile(int profile);

/*
 * get_media_max_profile: get media domain max profile index
 * return:max profile index value
 */
int get_media_max_profile(void);

/*
 * get_media_profile: get media domain profile index
 * return:profile index value
 */
int get_media_profile(void);

/*
 * set_media_profile: set media domain profile index
 * parameter:profile index
 * return:0:ok;   -1:fail
 */
int set_media_profile(int profile);

/*
 * set_media_voltage: set media domain voltage
 * parameter:set_volt(uV)
 * return:0:ok;   -1:fail
 */
int set_media_voltage(int set_volt);

/*
 * pm_init:init pm
 */
void pm_init(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif/* End of #ifndef __PM_DEVICE_H__*/

