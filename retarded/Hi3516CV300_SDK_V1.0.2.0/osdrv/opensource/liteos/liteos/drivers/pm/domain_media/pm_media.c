#include "pm_media.h"
#include "pm_media_regulator.h"
#include "pm_media_avs.h"

struct pm_domain_device media_domain_dev = {
    .domain_regulator_init  = media_domain_regulator_init,
    .domain_avs_init        = media_domain_avs_init,
};

