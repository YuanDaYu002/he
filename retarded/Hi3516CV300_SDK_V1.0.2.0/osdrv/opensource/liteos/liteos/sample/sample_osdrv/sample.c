/*
 *  osdrv sample
 */
#include "sys/types.h"
#include "sys/time.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/statfs.h"
#include "limits.h"

#include "los_event.h"
#include "los_printf.h"

#include "lwip/tcpip.h"
#include "lwip/netif.h"
#if defined(LOSCFG_DRIVERS_HIGMAC) || defined(LOSCFG_DRIVERS_HIETH_SF)
#include "eth_drv.h"
#endif
#include "arch/perf.h"

#include "fcntl.h"
#include "fs/fs.h"

#include "stdio.h"

#include "shell.h"
#include "hisoc/uart.h"
#include "vfs_config.h"
#include "disk.h"

#include "los_cppsupport.h"

#include "linux/fb.h"
#include "console.h"
#include "uart.h"
#include "implementation/usb_init.h"
#include "securec.h"
int secure_func_register(void)
{
    int ret;
    STlwIPSecFuncSsp stlwIPSspCbk= {0};
    stlwIPSspCbk.pfMemset_s = memset_s;
    stlwIPSspCbk.pfMemcpy_s = memcpy_s;
    stlwIPSspCbk.pfStrNCpy_s = strncpy_s;
    stlwIPSspCbk.pfStrNCat_s = strncat_s;
    stlwIPSspCbk.pfStrCat_s = strcat_s;
    stlwIPSspCbk.pfMemMove_s = memmove_s;
    stlwIPSspCbk.pfSnprintf_s = snprintf_s;
    stlwIPSspCbk.pfRand = rand;
    ret = lwIPRegSecSspCbk(&stlwIPSspCbk);
    if (ret != 0)
    {
        PRINT_ERR("\n***lwIPRegSecSspCbk Failed***\n");
        return -1;
    }

    PRINTK("\nCalling lwIPRegSecSspCbk\n");
    return ret;
}

extern UINT32 osShellInit(char *);

struct netif *pnetif;

void net_init()
{
#ifdef LOSCFG_DRIVERS_HIGMAC
    extern struct los_eth_driver higmac_drv_sc;
    pnetif = &(higmac_drv_sc.ac_if);
    higmac_init();
#endif

#ifdef LOSCFG_DRIVERS_HIETH_SF
    extern struct los_eth_driver hisi_eth_drv_sc;
    pnetif = &(hisi_eth_drv_sc.ac_if);
    if(hisi_eth_init()) {
        dprintf("hisi_eth_init init fail!!!\n");
        return;
    }
#endif

    netif_set_up(pnetif);
}


//#define SUPPORT_FMASS_PARITION
#ifdef SUPPORT_FMASS_PARITION
extern int fmass_register_notify(void(*notify)(void* context, int status), void* context);
extern int fmass_partition_startup(char* path);
void fmass_app_notify(void* conext, int status)
{
    if(status == 1)/*usb device connect*/
    {
        char *path = "/dev/mmcblk0p0";
        //startup fmass access patition
        fmass_partition_startup(path);
    }
}
#endif

#include "board.h"
extern UINT32 g_sys_mem_addr_end;
extern unsigned int g_uart_fputc_en;
void board_config(void)
{
    g_sys_mem_addr_end = SYS_MEM_BASE + SYS_MEM_SIZE_DEFAULT;
    g_uart_fputc_en = 1;
#ifdef LOSCFG_DRIVERS_USB
    extern unsigned long g_usb_mem_addr_start;
    g_usb_mem_addr_start = g_sys_mem_addr_end;
#endif

#ifndef TWO_OS
    //different board should set right mode:"rgmii" "rmii" "mii"
    //if you don't set :
    //hi3516a board's default mode is "rgmii"
    //hi3518ev200 board's default mode is "rmii"
    //hi3519 board's default mode is "rgmii"
#if defined(HI3516A) || defined(HI3519) || defined(HI3519V101) || defined(HI3559) || defined(HI3556)
    hisi_eth_set_phy_mode("rgmii");
#endif
#if defined(HI35168EV200)
    hisi_eth_set_phy_mode("rmii");
#endif
    //different board should set right addr:0~31
    //if you don't set ,driver will detect it automatically
    //hisi_eth_set_phy_addr(0);//0~31

#if (defined(HI3518EV200) &&defined(LOSCFG_DRIVERS_EMMC)) || defined(HI3519) || defined(HI3519V101) || defined(HI3559) || defined(HI3556) || defined(HI3516CV300)
    size_t part0_start_sector = 16   * (0x100000/512);
    size_t part0_count_sector = 1024 * (0x100000/512);
    size_t part1_start_sector = 16   * (0x100000/512) + part0_count_sector;
    size_t part1_count_sector = 1024 * (0x100000/512);
    extern struct disk_divide_info emmc;
    add_mmc_partition(&emmc, part0_start_sector, part0_count_sector);
    add_mmc_partition(&emmc, part1_start_sector, part1_count_sector);
#endif
#endif
}

void app_init(void)
{
    UINT32 uwRet;

    dprintf("uart init ...\n");
    uart_dev_init();


    dprintf("spi bus init ...\n");
    spi_dev_init();
#ifdef LOSCFG_DRIVERS_HIDMAC
    dprintf("dmac init\n");
    dmac_init();
#endif
    dprintf("i2c bus init ...\n");
    i2c_dev_init();

    dprintf("random dev init ...\n");
    ran_dev_register();
#if defined(LOSCFG_HW_RANDOM_ENABLE)
    dprintf("hw random dev init ...\n");
    random_hw_dev_register();
#endif

    dprintf("mem dev init ...\n");
    mem_dev_register();

    dprintf("porc fs init ...\n");
    proc_fs_init();

    dprintf("cxx init ...\n");
    extern char __init_array_start__, __init_array_end__;
    LOS_CppSystemInit((unsigned long)&__init_array_start__, (unsigned long)&__init_array_end__, NO_SCATTER);

#ifndef TWO_OS
#ifdef LOSCFG_DRIVERS_VIDEO
    dprintf("fb dev init ...\n");
    struct fb_info *info = malloc(sizeof(struct fb_info));
    register_framebuffer(info);
#endif
    dprintf("nand init ...\n");
    if(!nand_init()) {
        add_mtd_partition("nand", 0x200000, 32*0x100000, 0);
        add_mtd_partition("nand", 0x200000 + 32*0x100000, 32*0x100000, 1);
        mount("/dev/nandblk0", "/yaffs0", "yaffs", 0, NULL);
        //mount("/dev/nandblk1", "/yaffs1", "yaffs", 0, NULL);
    }

    dprintf("spi nor flash init ...\n");
    if(!spinor_init()){
        add_mtd_partition("spinor", 0x100000, 2*0x100000, 0);
        add_mtd_partition("spinor", 3*0x100000, 2*0x100000, 1);
#ifndef HI3559
        mount("/dev/spinorblk0", "/jffs0", "jffs", 0, NULL);
#endif
        //mount("/dev/spinorblk1", "/jffs1", "jffs", 0, NULL);
    }

    dprintf("gpio init ...\n");
    gpio_dev_init();

    dprintf("sd/mmc host init ...\n");
    SD_MMC_Host_init();
    msleep(2000);
#ifndef SUPPORT_FMASS_PARITION
    uwRet = mount("/dev/mmcblk0p0", "/sd0p0", "vfat", 0, 0);
    if (uwRet)
        dprintf("mount mmcblk0p0 to sd0p0 err %d\n", uwRet);
    uwRet = mount("/dev/mmcblk0p1", "/sd0p1", "vfat", 0, 0);
    if (uwRet)
        dprintf("mount mmcblk0p1 to sd0p1 err %d\n", uwRet);
    uwRet = mount("/dev/mmcblk1p0", "/sd1p0", "vfat", 0, 0);
    if (uwRet)
        dprintf("mount mmcblk1p0 to sd1p0 err %d\n", uwRet);
    uwRet = mount("/dev/mmcblk2p0", "/sd2p0", "vfat", 0, 0);
    if (uwRet)
        dprintf("mount mmcblk2p0 to sd2p0 err %d\n", uwRet);
#endif
    msleep(2000);
    uwRet = mount("/dev/sdap0", "/usbp0", "vfat", 0, 0);
    if (uwRet)
        dprintf("mount sdap0 to usbp0 err %d\n", uwRet);
    uwRet = mount("/dev/sdap1", "/usbp1", "vfat", 0, 0);
    if (uwRet)
        dprintf("mount sdap1 to usbp1 err %d\n", uwRet);

#ifdef HI3516A
    dprintf("dvfs init ...\n");
    dvfs_init();
#endif
#endif

    dprintf("tcpip init ...\n");
    (void)secure_func_register();
    tcpip_init(NULL, NULL);

#ifdef LOSCFG_DRIVERS_USB
    dprintf("usb init ...\n");
#ifdef SUPPORT_FMASS_PARITION
    //g_usb_mem_addr_start  must be set in board_config
    //usb_init must behind SD_MMC_Host_init
    uwRet = usb_init(DEVICE, DEV_MASS);
#else
    uwRet = usb_init(HOST, 0);
#endif
#endif

#ifdef SUPPORT_FMASS_PARITION
    fmass_register_notify(fmass_app_notify,NULL);
#endif

    dprintf("net init ...\n");
    net_init();

    dprintf("shell init ...\n");
    system_console_init(TTY_DEVICE);
    osShellInit(TTY_DEVICE);

    dprintf("g_sys_mem_addr_end=0x%08x,\n",g_sys_mem_addr_end);
    dprintf("done init!\n");
    dprintf("Date:%s.\n", __DATE__);
    dprintf("Time:%s.\n", __TIME__);

#ifdef TWO_OS
#ifdef LOSCFG_DRIVERS_PM
    dprintf("pm init...\n");
    pm_init();
#endif
    extern int _ipcm_vdd_init(void);
    extern int ipcm_net_init(void);

    dprintf("_ipcm vdd init ...\n");
    _ipcm_vdd_init();

    dprintf("ipcm net init ...\n");
    ipcm_net_init();

    extern int HI_ShareFs_Client_Init(char *);
    dprintf("share fs init ... \n");
    HI_ShareFs_Client_Init("/liteos");
    dprintf("share fs init done.\n");
#endif

#ifdef __OSDRV_TEST__
#ifndef TWO_OS
    msleep(5000);
#endif
    test_app_init();
#endif

    return;
}

