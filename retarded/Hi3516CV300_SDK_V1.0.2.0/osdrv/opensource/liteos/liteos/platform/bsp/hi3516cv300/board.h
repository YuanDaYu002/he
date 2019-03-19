#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* sample.bin should be loaded at SYS_MEM_BASE */

#define SYS_MEM_BASE    0x80000000

//#define SYS_MEM_SIZE_DEFAULT 0x2000000		//32M---> MMZ:31M
//#define SYS_MEM_SIZE_DEFAULT 0x1E00000	  	//30M---> MMZ:33M
//#define SYS_MEM_SIZE_DEFAULT 0x1D00000	  	//29M---> MMZ:34M
//#define SYS_MEM_SIZE_DEFAULT 0x1C00000	  	//28M---> MMZ:35M
//#define SYS_MEM_SIZE_DEFAULT 0x1B00000	  	//27M---> MMZ:36M
//#define SYS_MEM_SIZE_DEFAULT 0x1A00000	  	//26M---> MMZ:37M
//#define SYS_MEM_SIZE_DEFAULT 0x1800000	  	//24M---> MMZ:39M
//#define SYS_MEM_SIZE_DEFAULT 0x1700000	  	//23M---> MMZ:40M
//#define SYS_MEM_SIZE_DEFAULT 0x1600000	  	//22M---> MMZ:41M
//#define SYS_MEM_SIZE_DEFAULT 0x1500000	  	//21M---> MMZ:42M
//#define SYS_MEM_SIZE_DEFAULT 0x1400000	  	//20M---> MMZ:43M
#define SYS_MEM_SIZE_DEFAULT 0x1300000	  	//19M---> MMZ:44M
//#define SYS_MEM_SIZE_DEFAULT 0x1200000	  	//18M---> MMZ:45M

#define USB_MEM_SIZE         0x100000

#define DDR_MEM_SIZE            0x4000000ULL
#define MMZ_MEM_BASE_OFFSET     0

#define TEXT_OFFSET     0x0

#if ((SYS_MEM_SIZE_DEFAULT & 0xfffff) || (USB_MEM_SIZE & 0xfffff))
#error Please check config:SYS_MEM_SIZE_DEFAULT, USB_MEM_SIZE should be aligan  1M!
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
