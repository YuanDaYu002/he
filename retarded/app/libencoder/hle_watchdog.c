
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

//#include "ssp.h"
#include "mpi_sys.h"
#include "los_hwi.h"
#include "los_typedef.h"
#include "system_upgrade.h"
#include "hi_osal.h"
#include "typeport.h"
#include "hle_watchdog.h"

/* define watchdog IO */
#define HIWDT_BASE      0x12080000
#define HIWDT_REG(x)    (HIWDT_BASE + (x))

#define HIWDT_LOAD      0x000
#define HIWDT_VALUE     0x004
#define HIWDT_CTRL      0x008
#define HIWDT_INTCLR    0x00C
#define HIWDT_RIS       0x010
#define HIWDT_MIS       0x014
#define HIWDT_LOCK      0xC00

#define HIWDT_UNLOCK_VAL    0x1ACCE551

#define WATCHDOG_MINOR    101


extern volatile void *gpWtdgAllReg;
#define IO_WDT_ADDRESS(x) ((unsigned long)(gpWtdgAllReg) + ((x)-(HIWDT_BASE)))
#define hiwdt_readl(x)      osal_readl(IO_WDT_ADDRESS(HIWDT_REG(x)))
#define hiwdt_writel(v,x)   osal_writel(v, IO_WDT_ADDRESS(HIWDT_REG(x)))


#define WATCHDOG_IOCTL_BASE		'W'
#define WDIOC_SETTIMEOUT		_IOWR(WATCHDOG_IOCTL_BASE, 6, int)
#define WDIOC_SETOPTIONS		_IOWR(WATCHDOG_IOCTL_BASE, 4, int)
#define WDIOC_KEEPALIVE			_IO(WATCHDOG_IOCTL_BASE, 5)
#define WDIOS_DISABLECARD		0x0001    /* Turn off the watchdog timer */
#define WDIOS_ENABLECARD		0x0002    /* Turn on the watchdog timer */

#define WDG_DEV_FILE	"/dev/watchdog"

static struct osal_spinlock hidog_lock;

static int options = WDIOS_ENABLECARD;

extern int wtdg_mod_init(void *pArgs);
extern void wtdg_mod_exit(void);


/*******************************************************************************
*@ Description    :看门狗中断响应函数
*@ Input          :
*@ Output         :
*@ Return         :
*@ attention      :
*******************************************************************************/
static void watchdog_irqhandle(void)
{
	ERROR_LOG("watchdog interrupt!\n");
	//set_boot_region_bad();	//调试，注释掉
}


/*******************************************************************************
*@ Description    :注册中断响应函数
*@ Input          :
*@ Output         :
*@ Return         :
*@ attention      :
*******************************************************************************/
static void watchdog_interrupt_func_init(void)
{
	int a = 1;
	UINTPTR uvIntSave;
	//uvIntSave = LOS_IntLock(); //关中断
	LOS_HwiCreate(NUM_HAL_INTERRUPT_WDT, 0xa0, 0, (HWI_PROC_FUNC)watchdog_irqhandle, 0);//创建中断
	hal_interrupt_unmask(NUM_HAL_INTERRUPT_WDT); //中断使能
	//LOS_IntRestore(uvIntSave);//恢复到关中断之前的状态

	//hal_interrupt_mask(NUM_HAL_INTERRUPT_WDT);//中断屏蔽

}


/*******************************************************************************
*@ Description    :看门狗模块初始化
*@ Input          :
*@ Output         :
*@ Return         :
*@ attention      :
*******************************************************************************/
int hle_watchdog_init(void)
{
	wtdg_mod_init(NULL);
	watchdog_interrupt_func_init();
	return 0;
}

/*******************************************************************************
*@ Description    :喂狗函数
*@ Input          :
*@ Output         :
*@ Return         :成功 ： 0
*@ attention      :
*******************************************************************************/
int hle_watchdog_feed(void)
{
    unsigned long flags; 

    osal_spin_lock_irqsave(&hidog_lock, &flags); 
    /* unlock watchdog registers */ 
    hiwdt_writel(HIWDT_UNLOCK_VAL, HIWDT_LOCK); 
    /* clear watchdog */ 
    hiwdt_writel(0x00, HIWDT_INTCLR); 
    /* lock watchdog registers */ 
    hiwdt_writel(0, HIWDT_LOCK); 
    osal_spin_unlock_irqrestore(&hidog_lock, &flags); 

	return 0;
}

/*******************************************************************************
*@ Description    :退出看门狗
*@ Input          :
*@ Output         :
*@ Return         :
*@ attention      :
*******************************************************************************/
int hle_watchdog_exit(void)
{
	wtdg_mod_exit();
	return 0;
}











