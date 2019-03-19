#include "los_cpup.h"
#include "los_swtmr.h"
#include "los_base.h"
#include "los_mux.h"
#include "los_task.ph"
#include "asm/delay.h"
#include "asm/string.h"

#include "pm_device.h"

#include "pm_regulator.h"
#include "pm_dvfs.h"
#include "pm_avs.h"

#include "pm_cpu.h"
#include "pm_media.h"

#include "pm_proc.h"

struct pm_device pm_dev;

static void pm_domain_register(void)
{
    pm_dev.domain_num = 0;

    pm_dev.cpu_domain_device = &cpu_domain_dev;
    pm_dev.domain_num++;

    pm_dev.media_domain_device = &media_domain_dev;
    pm_dev.domain_num++;
}

static void pm_handle(UINT32 uwPara)
{
    if(!pm_dev.pm_enable)
        return;

    (void)LOS_MuxPend(pm_dev.lock,LOS_WAIT_FOREVER);

    if (pm_dev.cpu_domain_device->freq_dev->handle()) {
        goto out;
    }
    pm_dev.cpu_domain_device->avs_dev->handle();
    pm_dev.media_domain_device->avs_dev->handle();

out:
    (void)LOS_MuxPost(pm_dev.lock);
    return;
}

#ifndef SWTMR_TRIGGER
static void pm_task_handle(UINT32 uwPara)
{
    while(1) {
        pm_handle(0);
        /* interval=100ms */
        msleep(100);
    }
}
#endif

static void pm_timer_init(void)
{
    UINT32 uwRet = 0;
#ifdef SWTMR_TRIGGER
    UINT16 usSwTmrID = 0;

    /* interval=100ms */
    uwRet = LOS_SwtmrCreate(10, LOS_SWTMR_MODE_PERIOD, (SWTMR_PROC_FUNC)pm_handle, &usSwTmrID, 0);
    if(0 != uwRet){
        dprintf("LOS_SwtmrCreate pm_handle err.\n");
        return;
    }
    uwRet = LOS_SwtmrStart(usSwTmrID);
    if(0 != uwRet){
        dprintf("LOS_SwtmrStart  err.\n");
        return;
    }
#else
    TSK_INIT_PARAM_S pm_task;
    UINT32 pm_taskid;
    memset(&pm_task, 0, sizeof(TSK_INIT_PARAM_S));
    pm_task.pfnTaskEntry = (TSK_ENTRY_FUNC)pm_task_handle;
    pm_task.uwStackSize = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    pm_task.pcName = "pm_task";
    pm_task.usTaskPrio = 2;
    pm_task.uwResved = LOS_TASK_STATUS_DETACHED;
    uwRet = LOS_TaskCreate(&pm_taskid, &pm_task);
    if (LOS_OK != uwRet) {
        dprintf("Error:create pm_task failed!\n");
        return;
    }
#endif
}

/*
 * pm_init:init pm
 */
void pm_init(void)
{
    pm_domain_register();

    pm_regulator_init(&pm_dev);
    pm_dvfs_init(&pm_dev);
    pm_avs_init(&pm_dev);

    (void)LOS_MuxCreate(&(pm_dev.lock));
    pm_dev.pm_enable = 1;

    pm_timer_init();

    pm_proc_init();

    dprintf("pm init ok!.\n");
}

