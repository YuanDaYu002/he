#include "pm_cpu_regulator.h"
#include "pm_cpu_freq.h"
#include "pm_cpu_avs.h"

struct pm_domain_device cpu_domain_dev = {
    .domain_regulator_init  = cpu_domain_regulator_init,
    .domain_freq_init       = cpu_domain_freq_init,
    .domain_avs_init        = cpu_domain_avs_init
};

