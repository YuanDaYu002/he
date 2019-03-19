#include "los_printf.h"

#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "eth_drv.h"
#include "arch/perf.h"

#include "fcntl.h"
#include "fs/fs.h"

#include "stdio.h"

#include "shell.h"
#include "hisoc/uart.h"
#include "vfs_config.h"
#include "disk.h"

#include "los_cppsupport.h"

#include "los_scatter.h"
#include "board.h"
extern UINT32 g_sys_mem_addr_end;
extern unsigned int g_uart_fputc_en;
void board_config(void)
{
	g_sys_mem_addr_end = SYS_MEM_BASE + SYS_MEM_SIZE_DEFAULT;
	g_uwSysClock = OS_SYS_CLOCK;
	g_uart_fputc_en = 1;
#ifndef TWO_OS
	extern unsigned long g_usb_mem_addr_start;
	g_usb_mem_addr_start = g_sys_mem_addr_end;

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

#if (defined(HI3518EV200) &&defined(LOSCFG_DRIVERS_EMMC)) || defined(HI3519) || defined(HI3519V101) || defined(HI3559) || defined(HI3556)
	size_t part0_start_sector = 16	 * (0x100000/512);
	size_t part0_count_sector = 1024 * (0x100000/512);
	size_t part1_start_sector = 16	 * (0x100000/512) + part0_count_sector;
	size_t part1_count_sector = 1024 * (0x100000/512);
	extern struct disk_divide_info emmc;
	add_mmc_partition(&emmc, part0_start_sector, part0_count_sector);
	add_mmc_partition(&emmc, part1_start_sector, part1_count_sector);
#endif
#endif
}



void misc_driver_init()
{
    dprintf("random init ...\n");
    ran_dev_register();

    dprintf("uart init ...\n");
    uart_dev_init();

    dprintf("shell init ...\n");
    system_console_init(TTY_DEVICE);

    

    dprintf("spi nor falsh init ...\n");

    if (spinor_init())
    {
        dprintf("spi nor falsh init failed...\n");
    }

    dprintf("spi bus init ...\n");
    spi_dev_init();

    dprintf("i2c bus init ...\n");
    i2c_dev_init();

    dprintf("gpio init ...\n");
    gpio_dev_init();

    dprintf("dmac init\n");
    dmac_init();


    dprintf("usb init ...\n");


    dprintf("sd/mmc host init ...\n");
    SD_MMC_Host_init();
    msleep(500);
	


    dprintf("g_sys_mem_addr_end=0x%08x,\n", g_sys_mem_addr_end);
    dprintf("done init!\n");
    dprintf("Date:%s.\n", __DATE__);
    dprintf("Time:%s.\n", __TIME__);
}

