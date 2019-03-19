#include "pm_dvfs.h"

void pm_dvfs_init(struct pm_device *pm_dev)
{
    if(pm_dev->cpu_domain_device && pm_dev->cpu_domain_device->domain_freq_init)
        pm_dev->cpu_domain_device->domain_freq_init(pm_dev->cpu_domain_device);

    if(pm_dev->media_domain_device && pm_dev->media_domain_device->domain_freq_init)
        pm_dev->media_domain_device->domain_freq_init(pm_dev->media_domain_device);
}

