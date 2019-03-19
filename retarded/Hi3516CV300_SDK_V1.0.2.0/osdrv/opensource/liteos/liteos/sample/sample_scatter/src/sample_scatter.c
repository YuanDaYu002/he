#include "sys/types.h"
#include "sys/time.h"
#include "unistd.h"

#include "los_printf.h"
#include "stdio.h"
#include "shell.h"
#include "los_scatter.h"
#include "los_cppsupport.h"
#include "hisoc/uart.h"
#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "eth_drv.h"





static int image_read(void* memaddr, unsigned long start, unsigned long size)
{
    return hispinor_read(memaddr, start, size);
}

struct netif *pnetif;


void app_init(void)
{
    misc_driver_init();
    //TODO:
    //User can add code here. This part is loaded in  uboot.
#ifdef CFG_SCATTER_FLAG
    LOS_ScatterLoad(0x100000, image_read, 1);//"0x100000" is the address where the app is burned. "1" is spi flash  read unit
#endif
#ifndef CFG_FAST_IMAGE
    //TODO:
    //User can add code here. This part is loaded in  liteos.
    //Now I use the shell as an example.
    osShellInit(TTY_DEVICE);
    //shell_cmd_register();
	//net_init();
	

    dprintf("------------------start finish------------------ \n");
#endif

    while (1)
    {
    	//dprintf("------------------start finish------------------ \n");
        sleep(1);
    }
}


