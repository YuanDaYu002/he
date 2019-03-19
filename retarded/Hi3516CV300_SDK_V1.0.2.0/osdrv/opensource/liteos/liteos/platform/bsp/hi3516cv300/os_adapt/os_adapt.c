/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/
#include "los_base.h"
#include "los_event.h"
#include "mmu_config.h"
#ifdef LOSCFG_KERNEL_RUNSTOP
#include "los_runstop.h"
#endif
#ifdef LOSCFG_KERNEL_SCATTER
#include "los_scatter.h"
#endif
#ifdef LOSCFG_KERNEL_CPPSUPPORT
#include "los_cppsupport.h"
#endif
#ifdef LOSCFG_LIB_LIBC
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "sys/time.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/statfs.h"
#include "sys/stat.h"
#include "sys/mount.h"
#include "dirent.h"
#include "securec.h"
#endif /* LOSCFG_LIB_LIBC */
#ifdef LOSCFG_FS_VFS
#include "fs/fs.h"
#endif
#if defined(LOSCFG_FS_YAFFS) || defined(LOSCFG_FS_JFFS)
#include "mtd_partition.h"
#endif
#ifdef LOSCFG_FS_PROC
#include "proc_fs.h"
#endif
#ifdef LOSCFG_DRIVERS_UART
#include "console.h"
#include "uart_dev.h"
#endif
#ifdef LOSCFG_NET_LWIP_SACK
#include "lwip/tcpip.h"
#ifdef LOSCFG_DRIVERS_HIETH_SF
#include "eth_drv.h"
#endif
#endif
#ifdef LOSCFG_DRIVERS_MTD_NAND
#include "nand.h"
#endif
#ifdef LOSCFG_SHELL
#include "shell.h"
#include "shcmd.h"
#endif

#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR
#include "linux/mtd/mtd.h"
#endif

#ifdef LOSCFG_KERNEL_CPUP
#include "los_cpup.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR
extern struct mtd_info *spinor_mtd;
extern int spinor_init(void);
#endif
#ifdef LOSCFG_SHELL
extern unsigned int osShellInit(const char *);
#endif
#ifdef LOSCFG_KERNEL_SCATTER
extern char __fast_rodata_start, __fast_rodata_end;
extern char __fast_text_start, __fast_text_end;
#endif

extern char __text_start, __text_end;
extern char __rodata_start, __rodata_end;
extern char __rodata1_start, __rodata1_end;
extern char __init_array_start__, __init_array_end__;

extern int hinand_read(void* memaddr, unsigned long start, unsigned long size);
extern unsigned int GetTimer2Value(void);
extern void start_printf(const char *fmt, ...);

#ifdef LOSCFG_NET_LWIP_SACK
#ifdef LOSCFG_DRIVERS_HIETH_SF
extern struct los_eth_driver hisi_eth_drv_sc;
struct netif *pnetif_hi3516cv300 = &(hisi_eth_drv_sc.ac_if);

extern void hisi_eth_init(void);

void ipc_gmac_init(void)
{
    static unsigned int overtime = 0;
    PRINTK("Ethernet start.");

    hisi_eth_init();

    (void)netifapi_netif_set_up(pnetif_hi3516cv300);
    do {
        LOS_Msleep(60);
        overtime++;
        if (overtime > 100){
            PRINTK("netif_is_link_up overtime!\n");
            break;
        }
    } while(netif_is_link_up(pnetif_hi3516cv300) == 0);
}
#endif
#endif

void code_protect(void)
{
    extern SENCOND_PAGE stOsPage;
    MMU_PARAM mPara;

    /*note: must confirm that every addr be aglined as 4K(64K)*/
    PRINTK("__text_start = 0x%x, __text_end = 0x%x\n", &__text_start, &__text_end);
    PRINTK("__rodata_start = 0x%x, __rodata_end = 0x%x\n", &__rodata_start, &__rodata_end);
    PRINTK("__rodata1_start = 0x%x, __rodata1_end = 0x%x\n", &__rodata1_start, &__rodata1_end);

    mPara.startAddr = (unsigned long)&__text_start;
    mPara.endAddr = (unsigned long)&__rodata1_end;
    mPara.uwFlag = BUFFER_ENABLE|CACHE_ENABLE|ACCESS_PERM_RO_RO;
    mPara.stPage = (SENCOND_PAGE *)&stOsPage;

    LOS_MMUParamSet(&mPara);
}

void jffs2_fs_init(void)
{
#ifdef LOSCFG_FS_JFFS

    int uwRet = 0;

    if(uwRet = add_mtd_partition( "spinor", 0x100000, 0x200000, 0) != 0)
        PRINTK("add jffs partition failed, return %d\n", uwRet);

    PRINTK("Mount jffs2 on nor.\n");
    uwRet=mount("/dev/spinorblk0", "/jffs0", "jffs", 0, NULL);
    if(uwRet)
        PRINTK("mount jffs err %d\n",uwRet);
    PRINTK("Mount jffs2 on nor finished.\n");

#endif
}

void ram_fs_init(void)
{
#ifdef LOSCFG_FS_RAMFS
    int swRet=0;
    swRet = mount((const char *)NULL, RAMFS_MOUNT_POINT, "ramfs", 0, NULL);
    if (swRet) {
        PRINTK("mount ramfs err %d\n", swRet);
        return;
    }
#endif
    PRINTK("Mount ramfs finished.\n");
}



void rebind_yaffs(void)
{
}

void scatter_after(void)
{
#ifdef LOSCFG_FS_RAMFS
    ram_fs_init();
#endif
#ifdef LOSCFG_FS_PROC
    proc_fs_init();
#endif
#if defined (LOSCFG_FS_JFFS) && defined (LOSCFG_DRIVERS_MTD_SPI_NOR)
    if(!spinor_init())
    jffs2_fs_init();
#endif
#ifdef LOSCFG_FS_VFS
    if (mkdir("/bin",0) != 0)
        PRINTK("mkdir /bin failed\n");

    if (mkdir("/bin/vs",0) != 0)
        PRINTK("mkdir /bin/vs failed\n");

    if (mkdir("/bin/vs/sd",0) != 0)
        PRINTK("mkdir /bin/vs/sd failed\n");
#endif

/* MMU setting the permission access to Read-only memory */
#ifdef LOSCFG_KERNEL_SCATTER
    extern SENCOND_PAGE stOsPage;
    MMU_PARAM mPara;

    mPara.startAddr = (unsigned long)&__fast_rodata_start;
    mPara.endAddr = (unsigned long)&__fast_text_end;
    mPara.uwFlag = BUFFER_ENABLE|CACHE_ENABLE|ACCESS_PERM_RO_RO;
    mPara.stPage = (SENCOND_PAGE *)&stOsPage;

    LOS_MMUParamSet(&mPara);
#endif

#ifdef LOSCFG_KERNEL_CPPSUPPORT
    (VOID)LOS_CppSystemInit((UINT32)&__init_array_start__, (UINT32)&__init_array_end__, NO_SCATTER);
#endif
}
#ifdef LOSCFG_VENDOR
extern void hi_product_driver_init(void);
extern int vs_server(int s32Argc, char * apszArgv[]);
#endif


void wakeup_callback(void)
{
#ifdef LOSCFG_KERNEL_CPUP
    LOS_CpupReset();
#endif

    hal_interrupt_unmask(83);
#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR
    (void)spinor_init();
#endif
}

#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR
void spiflash_erase(unsigned long start, unsigned long size) {
    struct erase_info erase_test;
    memset(&erase_test, 0, sizeof(struct erase_info));
    erase_test.mtd = spinor_mtd;
    erase_test.callback = NULL; /*lint !e64*/
    erase_test.fail_addr = (uint64_t)MTD_FAIL_ADDR_UNKNOWN;
    erase_test.addr = start;
    erase_test.len = size;
    erase_test.time = 1;
    erase_test.retries = 1;
    erase_test.dev = 0;
    erase_test.cell = 0;
    erase_test.priv = 0;
    erase_test.state = 0;
    erase_test.next = (struct erase_info *)NULL;
    erase_test.scrub = 0;
    (void)spinor_mtd->erase(spinor_mtd, &erase_test);
}

int spiflash_write(void *memaddr, unsigned long start, unsigned long size) {
    size_t retlen;
    return spinor_mtd->write(spinor_mtd, start, size, &retlen, (const char *)memaddr);
}

int spiflash_read(void *memaddr, unsigned long start, unsigned long size) {
    size_t retlen;
    return spinor_mtd->read(spinor_mtd, start, size, &retlen, (const char *)memaddr);
}
#endif

#define NAND_ERASE_ALIGN_SIZE   (128 * 1024)
#define NAND_READ_ALIGN_SIZE    (2 * 1024)
#define NAND_WRITE_ALIGN_SIZE   (2 * 1024)

#define NOR_ERASE_ALIGN_SIZE    (64 * 1024)
#define NOR_READ_ALIGN_SIZE     (1)
#define NOR_WRITE_ALIGN_SIZE    (1)

#define EMMC_ERASE_ALIGN_SIZE   (512)
#define EMMC_READ_ALIGN_SIZE    (512)
#define EMMC_WRITE_ALIGN_SIZE   (512)


#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR
int flash_read(void *memaddr, unsigned long start, unsigned long size)
{
    return spiflash_read(memaddr, start, size);
}

int flash_write(void *memaddr, unsigned long start, unsigned long size)
{
    spiflash_erase(start, size);
    return spiflash_write(memaddr, start, size);
}
#endif

#ifdef LOSCFG_NET_LWIP_SACK
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
#endif

__attribute__((weak))  void app_init(void)
{
#ifdef LOSCFG_DRIVERS_RANDOM
    extern int ran_dev_register(void);
#endif
#ifdef LOSCFG_KERNEL_RUNSTOP
    RUNSTOP_PARAM_S stRunstopParam;
#endif
#ifdef LOSCFG_FS_PROC
    PRINTK("proc fs init ...\n");
    proc_fs_init();
#endif
#ifdef LOSCFG_DRIVERS_UART
    (void)hi_uartdev_init();
    if (system_console_init(TTY_DEVICE) != 0)
    {
        PRINT_ERR("system_console_init failed\n");
    }
#endif
#ifdef LOSCFG_DRIVERS_MTD_SPI_NOR
    (void)spinor_init();
#endif

#if defined (LOSCFG_KERNEL_RUNSTOP) && defined (LOSCFG_DRIVERS_MTD_SPI_NOR)
    memset(&stRunstopParam, 0, sizeof(RUNSTOP_PARAM_S));

    stRunstopParam.pfFlashReadFunc          = flash_read;
    stRunstopParam.pfFlashWriteFunc         = flash_write;
    stRunstopParam.pfImageDoneCallback      = NULL;
    stRunstopParam.pfWakeupCallback         = wakeup_callback;
    stRunstopParam.uwFlashEraseAlignSize    = NOR_ERASE_ALIGN_SIZE;
    stRunstopParam.uwFlashWriteAlignSize    = NOR_WRITE_ALIGN_SIZE;
    stRunstopParam.uwFlashReadAlignSize     = NOR_READ_ALIGN_SIZE;
    stRunstopParam.uwImageFlashAddr         = 0x100000;
    stRunstopParam.uwWowFlashAddr           = 0x800000;// 0x3000000;

    LOS_MakeImage(&stRunstopParam);
#endif

#ifndef MAKE_WOW_IMAGE

#ifdef LOSCFG_KERNEL_RUNSTOP
    extern UINT32 g_uwWowImgSize;
    PRINTK("Image length 0x%x\n", g_uwWowImgSize);
#endif

#ifdef LOSCFG_DRIVERS_RANDOM
    PRINTK("random init ...\n");
    (void)ran_dev_register();
#endif
#ifdef LOSCFG_DRIVERS_MEM
    extern int mem_dev_register(void);
    mem_dev_register();
#endif
#ifdef LOSCFG_DRIVERS_I2C
    extern int i2c_dev_init(void);
    PRINTK("i2c bus init ...\n");
    i2c_dev_init();
#ifdef LOSCFG_VENDOR
    PRINTK("CpuCore_Vol init ...\n");
    extern void CpuCore_Vol(void);
    CpuCore_Vol();
    PRINTK("CpuCore_Vol finish ...\n");
#endif
#endif

#if defined (LOSCFG_KERNEL_SCATTER) && defined (LOSCFG_DRIVERS_MTD_SPI_NOR)
    PRINTK("LOS_ScatterLoad...\n");
    LOS_ScatterLoad(0x100000, flash_read, NOR_READ_ALIGN_SIZE);
#endif

    code_protect();

#ifndef MAKE_SCATTER_IMAGE
#ifdef LOSCFG_SHELL
    (void)osShellInit(TTY_DEVICE);
#endif
#ifdef LOSCFG_NET_LWIP_SACK
    (void)secure_func_register();
#endif
#ifdef LOSCFG_VENDOR
    PRINTK("hi_product_driver_init ...\n");
    hi_product_driver_init();
    extern void SDK_init(void);
    PRINTK("sdk init ...\n");
    SDK_init();

    PRINTK("vs_server start ...\n");
    vs_server(0, NULL);
#endif

#endif /* MAKE_WOW_IMAGE */
#endif /* MAKE_SCATTER_IMAGE */
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
