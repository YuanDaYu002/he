#ifndef JFFS2_CONFIG_H
#define JFFS2_CONFIG_H

#include "spinor.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define JFFS_THREAD_PRIORITY_MAX       32
#define FILE_PATH_MAX                128  /* the longest file path */
#define CONFIG_JFFS2_ENTRY_NAME_MAX  256
#define JFFS2_NAME_MAX   CONFIG_JFFS2_ENTRY_NAME_MAX
#define JFFS2_PATH_MAX   FILE_PATH_MAX
#define JFFS_NOR_FLASH_BASE 0x100000
#define JFFS_NOR_FLASH_END_ADDR 0x2000000
#define JFFS_NOR_FLASH_BLOCK_SIZE (spinor_mtd->erasesize)
#define DEVICE_PART_MAX   1  /* the max partions on a nand deivce*/
#define JFFS2_PAGE_SIZE      1024
/* memory page size in kernel/asm/page.h, it is correspond with flash read/write
 * option, so this size has a great impact on reading/writing speed */
#define CONFIG_JFFS2_PAGE_SHIFT  12  /* (1<<12) 4096bytes*/


#define CONFIG_JFFS2_NO_RELATIVEDIR

#define JFFS2PKG_FS_JFFS2_RET_DIRENT_DTYPE
#if defined(JFFS2PKG_FS_JFFS2_RET_DIRENT_DTYPE)
    #define JFFS2PKG_FILEIO_DIRENT_DTYPE
#endif

#define JFFS2OPT_FS_JFFS2_WRITE /* if not defined, jffs2 is read only*/

/* jffs2 debug output opion */
#define CONFIG_JFFS2_FS_DEBUG       0  /* 1 or 2 */

/* jffs2 gc thread section */
#define JFFS2OPT_FS_JFFS2_GCTHREAD
#define JFFS2NUM_JFFS2_GC_THREAD_PRIORITY  (10) /* GC thread's priority */
#define JFFS2NUM_JFFS2_GS_THREAD_TICKS  20  /* event timeout ticks */
#define JFFS2NUM_JFFS2_GC_THREAD_TICKS  20  /* GC thread's running ticks */

//#define CONFIG_JFFS2_FS_WRITEBUFFER /* should not be enabled */

/* zlib section*/
#define CONFIG_JFFS2_ZLIB
#define CONFIG_JFFS2_RTIME
#define CONFIG_JFFS2_RUBIN
//#define CONFIG_JFFS2_CMODE_NONE
//#define CONFIG_JFFS2_CMODE_SIZE
#define SPIBLK_NAME "/dev/spinorblk"
#define SPICHR_NAME "/dev/spinorchr"

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
