-include $(LITEOSTOPDIR)/.config
ifeq ($(LOSCFG_COMPILER_LITEOS), y)
CROSS_COMPILE = arm-liteos-linux-uclibcgnueabi-
COMPILE_NAME = liteos
VERSION_NUM = 4.9.4
else ifeq ($(LOSCFG_COMPILER_HISIV500), y)
CROSS_COMPILE = arm-hisiv500-linux-
COMPILE_NAME = hisiv500
VERSION_NUM = 4.9.4
else ifeq ($(LOSCFG_COMPILER_HISIV300), y)
CROSS_COMPILE = arm-hisiv300-linux-uclibcgnueabi-
COMPILE_NAME = hisiv300
VERSION_NUM = 4.8.3
else ifeq ($(LOSCFG_COMPILER_HISIV600), y)
CROSS_COMPILE = arm-hisiv600-linux-gnueabi-
COMPILE_NAME = hisiv600
VERSION_NUM = 4.9.4
else ifeq ($(LOSCFG_COMPILER_HUAWEILITEOS), y)
CROSS_COMPILE = arm-himix100-linux-
COMPILE_NAME = himix100
VERSION_NUM = 6.3.0
endif

CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
AR = $(CROSS_COMPILE)ar
LD = $(CROSS_COMPILE)ld
GPP = $(CROSS_COMPILE)g++
OBJCOPY= $(CROSS_COMPILE)objcopy
OBJDUMP= $(CROSS_COMPILE)objdump

## platform relative ##
LITEOS_ARM_ARCH :=
LITEOS_CPU_TYPE :=
## c as cxx ld options ##
LITEOS_ASOPTS :=
LITEOS_COPTS_BASE :=
LITEOS_COPTS_EXTRA :=
LITEOS_COPTS_EXTRA_INTERWORK :=
LITEOS_COPTS :=
LITEOS_COPTS_NODEBUG :=
LITEOS_COPTS_INTERWORK :=
LITEOS_CXXOPTS :=
LITEOS_CXXOPTS_BASE :=
LITEOS_LD_OPTS :=
LITEOS_GCOV_OPTS :=
## dynload ld options ##
LITEOS_DYNLOADOPTS :=
## macro define ##
LITEOS_CMACRO :=
LITEOS_CMACRO_TEST :=
LITEOS_CXXMACRO :=
## head file path and ld path ##
LITEOS_INCLUDE :=
LITEOS_CXXINCLUDE :=
LITEOS_LD_PATH :=
LITEOS_LD_SCRIPT :=
LITEOS_MK_PATH :=
## c as cxx ld flags ##
LITEOS_ASFLAGS :=
LITEOS_CFLAGS :=
LITEOS_CFLAGS_INTERWORK :=
LITEOS_LDFLAGS :=
LITEOS_CXXFLAGS :=
## depended lib ##
LITEOS_BASELIB :=
LITEOS_SCATTERLIB :=
LITEOS_LIBDEP :=
## directory ##
LIB_BIGODIR :=
LIB_SUBDIRS :=
## mini system ##
LITEOS_MINISYS :=

## variable define ##
OUT = $(LITEOSTOPDIR)/out/$(LITEOS_PLATFORM)
BUILD = $(OUT)/obj
MK_PATH = $(LITEOSTOPDIR)/build/mk
CXX_PATH = $(LITEOSTOPDIR)/lib/cxxstl
JFFS_PATH = $(LITEOSTOPDIR)/fs/jffs2
LITEOS_SCRIPTPATH ?= $(LITEOSTOPDIR)/tools/scripts
LITEOS_LIB_BIGODIR = $(OUT)/lib/obj
LOSCFG_ENTRY_SRC   = $(LITEOSTOPDIR)/platform/bsp/$(LITEOS_PLATFORM)/config/los_config.c

ifeq ($(LOSCFG_COMPILER_HUAWEILITEOS), y)
LITEOS_CMACRO      += -D__COMPILER_HUAWEILITEOS__
endif
LITEOS_CMACRO      += -D__LITEOS__
LITEOS_SCATTERLIB  += -lgcc -lgcc_eh
AS_OBJS_LIBC_FLAGS  = -D__ASSEMBLY__

-include $(LITEOS_SCRIPTPATH)/scatter_sr/switch.mk
-include $(LITEOS_SCRIPTPATH)/scatter_sr/image.mk
-include $(LITEOSTOPDIR)/build/mk/dynload_ld.mk
####################################### Chip and CPU Option Begin #########################################
ifeq ($(LOSCFG_PLATFORM_HI3516A), y)
    LITEOS_CMACRO_TEST += -DTEST3516A
    AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
    LITEOS_PLATFORM    := hi3516a
    LITEOS_CPU_TYPE    := cortex-a7
    LITEOS_MINISYS     := n
    WARNING_AS_ERROR   :=
    LITEOS_CMACRO       += -DHI3516A
    LITEOS_CMACRO      += -DLOSCFG_CORTEX_A7

else ifeq ($(LOSCFG_PLATFORM_HI3518EV200), y)
    LITEOS_CMACRO_TEST += -DTEST3518E
    AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=5
    LITEOS_PLATFORM    := hi3518ev200
    LITEOS_CPU_TYPE    := arm926
    LITEOS_MINISYS     := n
    WARNING_AS_ERROR   :=
    LITEOS_CMACRO      += -DHI3518EV200
    LITEOS_CMACRO      += -DLOSCFG_ARM926

else ifeq ($(LOSCFG_PLATFORM_HI3516CV300), y)
    LITEOS_CMACRO_TEST += -DTEST3516CV300
    AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=5
    LITEOS_PLATFORM    := hi3516cv300
    LITEOS_CPU_TYPE    := arm926
    LITEOS_MINISYS     := n
    WARNING_AS_ERROR   :=
    LITEOS_CMACRO      += -DHI3516CV300
    LITEOS_CMACRO      += -DLOSCFG_ARM926

else ifeq ($(LOSCFG_PLATFORM_HIM5V100), y)
    LITEOS_CMACRO_TEST += -DTESTM5V100
    AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=5
    LITEOS_PLATFORM    := him5v100
    LITEOS_CPU_TYPE    := arm926
    LITEOS_MINISYS     := y
    WARNING_AS_ERROR   :=
    LITEOS_CMACRO      += -DLOSCFG_ARM926

else ifeq ($(LOSCFG_PLATFORM_HI3911), y)
    LITEOS_CMACRO_TEST += -DTEST3911
    AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=5
    LITEOS_PLATFORM    := hi3911
    LITEOS_CPU_TYPE    := arm926
    LITEOS_MINISYS     := y
    WARNING_AS_ERROR   :=
    LITEOS_CMACRO      += -DLOSCFG_ARM926

else ifeq ($(LOSCFG_PLATFORM_HI3519), y)
    ifeq ($(LOSCFG_PLATFORM_HI3519_CORTEX_A7), y)
        LITEOS_CMACRO_TEST += -DTEST3519
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
        LITEOS_PLATFORM    := hi3519/cortex-a7
        LITEOS_CPU_TYPE    := cortex-a7
        LITEOS_MINISYS     := y
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DLOSCFG_CORTEX_A7
        LITEOS_CMACRO      += -DHI3519
    else ifeq ($(LOSCFG_PLATFORM_HI3519_CORTEX_A17), y)
        LITEOS_CMACRO_TEST += -DTEST3519
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
        LITEOS_PLATFORM    := hi3519/cortex-a17
        LITEOS_CPU_TYPE    := cortex-a17
        LITEOS_MINISYS     := y
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DHI3519
        LITEOS_CMACRO      += -DLOSCFG_CORTEX_A17
    endif

else ifeq ($(LOSCFG_PLATFORM_HI3519V101), y)
    ifeq ($(LOSCFG_PLATFORM_HI3519V101_CORTEX_A7), y)
        LITEOS_CMACRO_TEST += -DTEST3519V101
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
        LITEOS_PLATFORM    := hi3519v101/cortex-a7
        LITEOS_CPU_TYPE    := cortex-a7
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DHI3519V101
        LITEOS_CMACRO      += -DLOSCFG_CORTEX_A7
    else ifeq ($(LOSCFG_PLATFORM_HI3519V101_CORTEX_A17), y)
        LITEOS_CMACRO_TEST += -DTEST3519V101
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
        LITEOS_PLATFORM    := hi3519v101/cortex-a17
        LITEOS_CPU_TYPE    := cortex-a17
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DHI3519V101
        LITEOS_CMACRO      += -DLOSCFG_CORTEX_A17
    endif

else ifeq ($(LOSCFG_PLATFORM_HI3559), y)
    ifeq ($(LOSCFG_PLATFORM_HI3559_CORTEX_A7), y)
        LITEOS_CMACRO_TEST += -DTEST3559
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
        LITEOS_PLATFORM    := hi3559/cortex-a7
        LITEOS_CPU_TYPE    := cortex-a7
		LITEOS_MINISYS     := n
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DHI3559
        LITEOS_CMACRO      += -DLOSCFG_CORTEX_A7
    else ifeq ($(LOSCFG_PLATFORM_HI3559_CORTEX_A17), y)
        LITEOS_CMACRO_TEST += -DTEST3559
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
        LITEOS_PLATFORM    := hi3559/cortex-a17
        LITEOS_CPU_TYPE    := cortex-a17
		LITEOS_MINISYS     := n
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DHI3559
        LITEOS_CMACRO      += -DLOSCFG_CORTEX_A17
        LITEOS_BASELIB    += -lipcm -lipcm_net -lsharefs
    endif
else ifeq ($(LOSCFG_PLATFORM_HI3556), y)
    ifeq ($(LOSCFG_PLATFORM_HI3556_CORTEX_A7), y)
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
        LITEOS_PLATFORM := hi3556/cortex-a7
        LITEOS_CPU_TYPE   := cortex-a7
        LITEOS_CMACRO       += -DHI3556
    	WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DLOSCFG_CORTEX_A7
    else ifeq ($(LOSCFG_PLATFORM_HI3556_CORTEX_A17), y)
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=7
        LITEOS_PLATFORM := hi3556/cortex-a17
        LITEOS_CPU_TYPE   := cortex-a17
        LITEOS_CMACRO       += -DHI3556 -DTWO_OS
        LITEOS_CMACRO      += -DLOSCFG_CORTEX_A17
    	WARNING_AS_ERROR   :=
        LITEOS_BASELIB    += -lipcm -lipcm_net -lsharefs
    endif
else ifeq ($(LOSCFG_PLATFORM_HI3731), y)
        LITEOS_CMACRO_TEST += -DTEST3731 -DLOSCFG_USB_ONLY_HOST_MODE -DMULTI_DEVICE
        LITEOS_CXXMACRO   += -DTEST3731
        AS_OBJS_LIBC_FLAGS += -D__LINUX_ARM_ARCH__=5
        LITEOS_PLATFORM    := hi3731
        LITEOS_CPU_TYPE    := arm926
        LITEOS_MINISYS     := y
        WARNING_AS_ERROR   :=
        LITEOS_CMACRO      += -DLOSCFG_ARM926

endif
####################################### Chip and CPU Option End #########################################

####################################### Kernel Option Begin ###########################################
ifeq ($(LOSCFG_KERNEL_LITEKERNEL), y)
    LITEOS_SCATTERLIB += -llitekernel
    LIB_SUBDIRS       += kernel/base
    LITEOS_KERNEL_INCLUDE   := -I $(LITEOSTOPDIR)/kernel/include
ifeq ($(LOSCFG_DEBUG_VERSION), y)
    LITEOS_CMACRO    +=  -DLOSCFG_MEM_WATERLINE
endif
endif
ifeq ($(LOSCFG_KERNEL_CPUP), y)
    LITEOS_SCATTERLIB   += -lcpup
    LIB_SUBDIRS         += kernel/extended/cpup
    LITEOS_CPUP_INCLUDE := -I $(LITEOSTOPDIR)/kernel/extended/cpup
    LITEOS_CMACRO       += -DLOSCFG_KERNEL_CPUP
endif

ifeq ($(LOSCFG_KERNEL_CPPSUPPORT), y)
    LITEOS_SCATTERLIB += -lsupc++
    LITEOS_BASELIB    += -lcppsupport -lstdc++
    LIB_SUBDIRS       += kernel/extended/cppsupport
    LITEOS_CPPSUPPORT_INCLUDE   += -I $(LITEOSTOPDIR)/kernel/extended/cppsupport

    LITEOS_CMACRO     += -DLOSCFG_KERNEL_CPPSUPPORT
    LITEOS_CXXMACRO   += -DLOSCFG_KERNEL_CPPSUPPORT

ifeq ($(LOSCFG_KERNEL_CPP_EXCEPTIONS_SUPPORT),y)
    LITEOS_CXXMACRO     += -DLOSCFG_KERNEL_CPP_EXCEPTIONS_SUPPORT
endif

endif

ifeq ($(LOSCFG_KERNEL_DYNLOAD), y)
    LITEOS_BASELIB   += -ldynload
    LIB_SUBDIRS      += kernel/extended/dynload
    LITEOS_DYNLOAD_INCLUDE   += -I $(LITEOSTOPDIR)/kernel/extended/dynload/include

    LITEOS_CMACRO    += -DLOSCFG_KERNEL_DYNLOAD
endif

ifeq ($(LOSCFG_KERNEL_RUNSTOP), y)
    LITEOS_SCATTERLIB += -lrunstop
    LIB_SUBDIRS       += kernel/extended/runstop
    LITEOS_RUNSTOP_INCLUDE   += -I $(LITEOSTOPDIR)/kernel/extended/runstop

    LITEOS_CMACRO     += -DLOSCFG_KERNEL_RUNSTOP
endif

ifeq ($(LOSCFG_KERNEL_SCATTER), y)
    LITEOS_SCATTERLIB += -lscatter
    LIB_SUBDIRS       += kernel/extended/scatter
    LITEOS_SCATTER_INCLUDE   += -I $(LITEOSTOPDIR)/kernel/extended/scatter

    LITEOS_CMACRO     += -DLOSCFG_KERNEL_SCATTER
endif
################################### Kernel Option End ################################

#################################### Lib Option Begin ###############################
ifeq ($(LOSCFG_LIB_LIBC), y)
    LITEOS_SCATTERLIB  += -lc -lcsysdeps -lsec
    LIB_SUBDIRS        += lib/libc
    LIB_SUBDIRS        += lib/libsec
    ifeq ($(LITEOS_CPU_TYPE), cortex-a7)
        LIB_SUBDIRS        += lib/libc/src/sysdeps/a7
        LITEOS_LIBC_INCLUDE += -I $(LITEOSTOPDIR)/lib/libc/kernel/uapi/asm-arm
        LITEOS_LIBC_INCLUDE += -I $(LITEOSTOPDIR)/lib/libc/arch-arm/include
    endif
    ifeq ($(LITEOS_CPU_TYPE), cortex-a17)
        LIB_SUBDIRS        += lib/libc/src/sysdeps/a7
        LITEOS_LIBC_INCLUDE += -I $(LITEOSTOPDIR)/lib/libc/kernel/uapi/asm-arm
        LITEOS_LIBC_INCLUDE += -I $(LITEOSTOPDIR)/lib/libc/arch-arm/include
    endif
    ifeq ($(LITEOS_CPU_TYPE), arm926)
        LIB_SUBDIRS        += lib/libc/src/sysdeps/common
        LITEOS_LIBC_INCLUDE += -I $(LITEOSTOPDIR)/lib/libc/kernel/uapi/asm-arm
        LITEOS_LIBC_INCLUDE += -I $(LITEOSTOPDIR)/lib/libc/arch-arm/include
    endif

    LITEOS_LIBC_INCLUDE   += \
        -I $(LITEOSTOPDIR)/lib/gcc/4.9.4/include \
        -I $(LITEOSTOPDIR)/lib/libc/include \
        -I $(LITEOSTOPDIR)/lib/libc/kernel/uapi \
        -I $(LITEOSTOPDIR)/lib/libsec/include

    LITEOS_CMACRO     +=  -DLOSCFG_LIB_LIBC -DCONFIG_STRERROR
endif


ifeq ($(LOSCFG_LIB_LIBCMINI), y)
    LITEOS_SCATTERLIB  += -lcmini
    LIB_SUBDIRS        += lib/libcmini
    LITEOS_LIBC_INCLUDE   += -I $(LITEOSTOPDIR)/lib/libcmini

    LITEOS_CMACRO      +=  -DLOSCFG_LIB_LIBCMINI
endif

ifeq ($(LOSCFG_LIB_LIBM), y)
    LITEOS_SCATTERLIB   += -lm
    LIB_SUBDIRS         += lib/libm
    LITEOS_LIBM_INCLUDE += -I $(LITEOSTOPDIR)/lib/libm/include
    LITEOS_CMACRO       +=  -DLOSCFG_LIB_LIBM
endif

ifeq ($(LOSCFG_LIB_ZLIB), y)
    LITEOS_BASELIB += -lz
    LIB_SUBDIRS    += lib/zlib
    LITEOS_ZLIB_INCLUDE += -I $(LITEOSTOPDIR)/lib/zlib/include
endif
################################### Lib Option End ######################################

####################################### Compat Option Begin #########################################
ifeq ($(LOSCFG_COMPAT_CMSIS), y)
    LITEOS_BASELIB  += -lcmsis
    LIB_SUBDIRS     += compat/cmsis
    LITEOS_CMSIS_INCLUDE += -I $(LITEOSTOPDIR)/compat/cmsis/include

    LITEOS_CMACRO   += -DLOSCFG_COMPAT_CMSIS
endif

ifeq ($(LOSCFG_COMPAT_POSIX), y)
    LITEOS_SCATTERLIB += -lposix
    LIB_SUBDIRS       += compat/posix
    LITEOS_POSIX_INCLUDE   += \
        -I $(LITEOSTOPDIR)/compat/posix/include \
        -I $(LITEOSTOPDIR)/compat/posix/src

    LITEOS_CMACRO     += -DLOSCFG_COMPAT_POSIX
endif

ifeq ($(LOSCFG_COMPAT_LINUX), y)
    LITEOS_SCATTERLIB += -llinuxadp
    LIB_SUBDIRS       += compat/linux
    LITEOS_LINUX_INCLUDE   += -I $(LITEOSTOPDIR)/compat/linux/include

    LITEOS_CMACRO     += -DLOSCFG_COMPAT_LINUX
endif
######################################## Compat Option End ############################################

################################## Frameworks Option Begin ##########################
ifeq ($(LOSCFG_FRAMEWORKS_M2MCOMMON), y)
    LITEOS_BASELIB += -lm2mcomm
    LIB_SUBDIRS    += frameworks/m2mcomm
endif
################################## Frameworks Option End ##########################


#################################### FS Option Begin ##################################
ifeq ($(LOSCFG_FS_VFS), y)
    LITEOS_SCATTERLIB += -lvfs
    LIB_SUBDIRS       += fs/vfs
    LITEOS_VFS_INCLUDE   += -I $(LITEOSTOPDIR)/fs/include
    LITEOS_VFS_INCLUDE   += -I $(LITEOSTOPDIR)/fs/vfs/include/driver
    LITEOS_BASELIB += -lmulti_partition
    LIB_SUBDIRS += fs/vfs/multi_partition
    LITEOS_VFS_MTD_INCLUDE := -I $(LITEOSTOPDIR)/fs/vfs/include/multi_partition
    LITEOS_CMACRO     += -DLOSCFG_FS_VFS
endif

ifeq ($(LOSCFG_FS_FAT), y)
    LITEOS_BASELIB  += -lfat
    LIB_SUBDIRS     += fs/fat

    LITEOS_CMACRO   += -DLOSCFG_FS_FAT
    LOSCFG_DRIVER_DISK := y
endif

ifeq ($(LOSCFG_FS_FAT_CACHE), y)
    LITEOS_BASELIB  += -lbcache
    LIB_SUBDIRS     += fs/vfs/bcache
    LITEOS_FAT_CACHE_INCLUDE += -I $(LITEOSTOPDIR)/fs/vfs/include/bcache

    LITEOS_CMACRO   += -DLOSCFG_FS_FAT_CACHE
endif

ifeq ($(LOSCFG_FS_FAT_CHINESE), y)
    LITEOS_CMACRO   += -DLOSCFG_FS_FAT_CHINESE
endif

ifeq ($(LOSCFG_FS_RAMFS), y)
    LITEOS_BASELIB  += -lramfs
    LIB_SUBDIRS     += fs/ramfs

    LITEOS_CMACRO   += -DLOSCFG_FS_RAMFS
endif

ifeq ($(LOSCFG_FS_YAFFS), y)
    LITEOS_BASELIB  += -lyaffs2
    LIB_SUBDIRS     += fs/yaffs2

    LITEOS_CMACRO   += -DLOSCFG_FS_YAFFS
endif

ifeq ($(LOSCFG_FS_YAFFS_CACHE), y)
    LITEOS_CMACRO   += -DLOSCFG_FS_YAFFS_CACHE
endif

ifeq ($(LOSCFG_FS_NFS), y)
    LITEOS_BASELIB  += -lnfs
    LIB_SUBDIRS     += fs/nfs

    LITEOS_CMACRO   += -DLOSCFG_FS_NFS
endif

ifeq ($(LOSCFG_FS_PROC), y)
    LITEOS_BASELIB  += -lproc
    LIB_SUBDIRS     += fs/proc
    LITEOS_PROC_INCLUDE += -I $(LITEOSTOPDIR)/fs/proc/include

    LITEOS_CMACRO   += -DLOSCFG_FS_PROC
endif


ifeq ($(LOSCFG_FS_JFFS), y)
    LITEOS_BASELIB  += -ljffs2
    LIB_SUBDIRS     += fs/jffs2

    LITEOS_CMACRO   += -DLOSCFG_FS_JFFS
endif
#################################### FS Option End ##################################


################################### Net Option Begin ###################################
ifeq ($(LOSCFG_NET_LWIP_SACK), y)
    LITEOS_SCATTERLIB += -llwip
    LITEOS_BASELIB    += -llwip
    LIB_SUBDIRS       += net/lwip_sack
    LITEOS_LWIP_SACK_INCLUDE   += \
        -I $(LITEOSTOPDIR)/net/lwip_sack/include \
        -I $(LITEOSTOPDIR)/net/lwip_sack/include/ipv4 \
        -I $(LITEOSTOPDIR)/net/mac

    LITEOS_CMACRO     +=  -DLOSCFG_NET_LWIP_SACK -DLWIP_BSD_API $(LWIP_MACROS)
ifeq ($(LOSCFG_DEBUG_VERSION), y)
    LITEOS_CMACRO    +=  -DLWIP_DEBUG
else
    LITEOS_CMACRO    +=  -DLWIP_NOASSERT
endif
endif

ifeq ($(LOSCFG_NET_LWIP_SACK_TFTP), y)
    LITEOS_CMACRO     += -DLOSCFG_NET_LWIP_SACK_TFTP
endif

ifeq ($(LOSCFG_NET_WPA), y)
    LITEOS_BASELIB    += -lwpa
    LIB_SUBDIRS       +=  net/wpa_supplicant-2.2
endif

ifeq ($(LOSCFG_NET_PCAP), y)
    LITEOS_BASELIB += -lpcap
    LIB_SUBDIRS += tools/pcap
endif
#################################### Net Option End####################################

################################## Driver Option Begin #################################

ifeq ($(LOSCFG_DRIVERS_GPIO), y)
    LITEOS_SCATTERLIB     += -lgpio
    LIB_SUBDIRS           += drivers/gpio
    LITEOS_GPIO_INCLUDE   += -I $(LITEOSTOPDIR)/drivers/gpio/include

    LITEOS_CMACRO         += -DLOSCFG_DRIVERS_GPIO
endif

ifeq ($(LOSCFG_DRIVERS_HIDMAC), y)
    LITEOS_BASELIB    += -lhidmac
    LIB_SUBDIRS       += drivers/hidmac
    LITEOS_HIDMAC_INCLUDE   += -I $(LITEOSTOPDIR)/drivers/hidmac/include

    LITEOS_CMACRO     += -DLOSCFG_DRIVERS_HIDMAC
endif

ifeq ($(LOSCFG_DRIVERS_HIETH_SF), y)
       LITEOS_HIETH_SF_INCLUDE += -I $(LITEOSTOPDIR)/drivers/hieth-sf/include
       LITEOS_BASELIB    += -lhieth-sf
       LIB_SUBDIRS       +=  drivers/hieth-sf

       LITEOS_CMACRO     +=  -DLOSCFG_DRIVERS_HIETH_SF
endif

ifeq ($(LOSCFG_DRIVERS_HIGMAC), y)
    LITEOS_HIGMAC_INCLUDE += -I $(LITEOSTOPDIR)/drivers/higmac/include
    LITEOS_BASELIB    += -lhigmac
    LIB_SUBDIRS       += drivers/higmac

    LITEOS_CMACRO     += -DLOSCFG_DRIVERS_HIGMAC
endif

ifeq ($(LOSCFG_DRIVERS_I2C), y)
    LITEOS_SCATTERLIB += -li2c
    LIB_SUBDIRS       += drivers/i2c
    LITEOS_I2C_INCLUDE   += -I $(LITEOSTOPDIR)/drivers/i2c/include

    LITEOS_CMACRO     += -DLOSCFG_DRIVERS_I2C
    LITEOS_CMACRO     += -DLOSCFG_I2C_ADAPTER_COUNT=$(LOSCFG_I2C_ADAPTER_COUNT)
ifeq ($(LOSCFG_HOST_TYPE_HIBVT), y)
		LITEOS_CMACRO 	+= -DLOSCFG_HOST_TYPE_HIBVT
endif
endif

ifeq ($(LOSCFG_DRIVERS_MEM), y)
    LITEOS_SCATTERLIB += -lmem
    LIB_SUBDIRS       += drivers/mem

    LITEOS_CMACRO     += -DLOSCFG_DRIVERS_MEM
endif

ifeq ($(LOSCFG_DRIVERS_MMC), y)
	MMC_HOST_DIR := himci

    LITEOS_MMC_INCLUDE += -I $(LITEOSTOPDIR)/drivers/mmc/include
    LITEOS_SCATTERLIB  += -lmmc
    LIB_SUBDIRS        += drivers/mmc

    LITEOS_CMACRO      += -DLOSCFG_DRIVERS_MMC
    LOSCFG_DRIVER_DISK := y
ifeq ($(LOSCFG_DRIVERS_EMMC), y)
	LITEOS_CMACRO      += -DLOSCFG_DRIVERS_EMMC
endif
ifeq ($(LOSCFG_DRIVERS_MMC_SPEEDUP), y)
	LITEOS_CMACRO      += -DLOSCFG_DRIVERS_MMC_SPEEDUP
endif
endif

ifeq ($(LOSCFG_DRIVERS_MTD), y)
    LITEOS_BASELIB    += -lmtd_common
    LIB_SUBDIRS       += drivers/mtd/common
    LITEOS_CMACRO     += -DLOSCFG_DRIVERS_MTD

    ifeq ($(LOSCFG_DRIVERS_MTD_NAND), y)
        ifeq ($(LOSCFG_DRIVERS_MTD_NAND_HINFC620), y)
            NAND_DRIVER_DIR := hinfc620
    else ifeq ($(LOSCFG_DRIVERS_MTD_NAND_HIFMC100), y)
            NAND_DRIVER_DIR := hifmc100
    else ifeq ($(LOSCFG_DRIVERS_MTD_NAND_HISNFC100), y)
            NAND_DRIVER_DIR := hisnfc100
    else ifeq ($(LOSCFG_DRIVERS_MTD_NAND_HIFMC100_PARALLEL), y)
            NAND_DRIVER_DIR := hifmc100_nand
    endif

        LITEOS_SCATTERLIB += -lnand_flash
        LIB_SUBDIRS       += drivers/mtd/nand
        LITEOS_MTD_NAND_INCLUDE  += -I $(LITEOSTOPDIR)/drivers/mtd/nand/include

        LITEOS_CMACRO     += -DLOSCFG_DRIVERS_MTD_NAND
    endif

    ifeq ($(LOSCFG_DRIVERS_MTD_SPI_NOR), y)
        ifeq ($(LOSCFG_DRIVERS_MTD_SPI_NOR_HISFC350), y)
            NOR_DRIVER_DIR := hisfc350
    else ifeq ($(LOSCFG_DRIVERS_MTD_SPI_NOR_HIFMC100), y)
            NOR_DRIVER_DIR := hifmc100
    endif

        LITEOS_BASELIB   += -lspinor_flash
        LIB_SUBDIRS      += drivers/mtd/spi_nor
        LITEOS_MTD_SPI_NOR_INCLUDE  +=  -I $(LITEOSTOPDIR)/drivers/mtd/spi_nor/include

        LITEOS_CMACRO    += -DLOSCFG_DRIVERS_MTD_SPI_NOR
    endif
endif

ifeq ($(LOSCFG_DRIVERS_RANDOM), y)
    LITEOS_BASELIB += -lrandom
    LIB_SUBDIRS    += drivers/random
    LITEOS_RANDOM_INCLUDE += -I $(LITEOSTOPDIR)/drivers/random/include
ifeq ($(LOSCFG_HW_RANDOM_ENABLE), y)
    LITEOS_CMACRO+= -DLOSCFG_HW_RANDOM_ENABLE
endif

    LITEOS_CMACRO   +=  -DLOSCFG_DRIVERS_RANDOM
endif

ifeq ($(LOSCFG_DRIVERS_RTC), y)
    LITEOS_SCATTERLIB  += -lrtc
    LIB_SUBDIRS        += drivers/rtc

    LITEOS_CMACRO      +=  -DLOSCFG_DRIVERS_RTC
endif

ifeq ($(LOSCFG_DRIVERS_SPI), y)
    LITEOS_BASELIB  +=  -lspi
    LIB_SUBDIRS     += drivers/spi
    LITEOS_SPI_INCLUDE += -I $(LITEOSTOPDIR)/drivers/spi/include

    LITEOS_CMACRO   += -DLOSCFG_DRIVERS_SPI
endif

ifeq ($(LOSCFG_DRIVERS_UART), y)
    LITEOS_SCATTERLIB   += -luart
    LITEOS_BASELIB      += -luart
    LIB_SUBDIRS         +=  drivers/uart
    LITEOS_UART_INCLUDE += -I $(LITEOSTOPDIR)/drivers/uart/include
    LITEOS_CMACRO       += -DLOSCFG_DRIVERS_UART
endif

ifeq ($(LOSCFG_DRIVERS_USB), y)
    LITEOS_BASELIB  += -lusb
    LIB_SUBDIRS     += drivers/usb
    LITEOS_USB_INCLUDE += -I $(LITEOSTOPDIR)/drivers/usb

    LITEOS_CMACRO   += -DLOSCFG_DRIVERS_USB -DSUPPORT_LOS_USB_NEW_DRIVER -DUSB_DEBUG_VAR=5 \
                       -DUSB_GLOBAL_INCLUDE_FILE=\"implementation/global_implementation.h\"
endif

ifeq ($(LOSCFG_DRIVERS_VIDEO), y)
    LITEOS_SCATTERLIB += -lvideo
    LIB_SUBDIRS       += drivers/video
endif

ifeq ($(LOSCFG_DRIVERS_WIFI_BCM), y)
    LITEOS_SCATTERLIB += -lwwd -lwifi_adapt
    LIB_SUBDIRS       += drivers/wifi/bcm_wifi  drivers/wifi/wifi_adapt

    LITEOS_CMACRO     += -DLOSCFG_DRIVERS_WIFI_BCM

else ifeq ($(LOSCFG_DRIVERS_WIFI_QRD), y)
    LITEOS_SCATTERLIB += -lar6003
    LIB_SUBDIRS       += drivers/wifi/ar6k3_wifi
endif

ifeq ($(LOSCFG_DRIVERS_WTDG), y)
    LITEOS_BASELIB +=  -lwtdg
    LIB_SUBDIRS    += drivers/wtdg
    LITEOS_WTDG_INCLUDE := -I $(LITEOSTOPDIR)/drivers/wtdg/include
endif
ifeq ($(LOSCFG_DRIVER_DISK), y)
    LITEOS_BASELIB += -ldisk
    LIB_SUBDIRS += fs/vfs/disk
    LITEOS_CMACRO       += -DLOSCFG_DRIVER_DISK
endif
############################## Driver Option End #######################################

############################# Tools && Debug Option Begin ##############################
ifeq ($(LOSCFG_COMPILE_DEBUG), y)
    LITEOS_ASOPTS   += -g -gdwarf-2
    LITEOS_COPTS    += -g -gdwarf-2 -O0
    LITEOS_COPTS_INTERWORK += -g -gdwarf-2 -O0
    LITEOS_CXXOPTS  += -g -gdwarf-2 -O0
else
    LITEOS_COPTS    += -O2
    LITEOS_COPTS_INTERWORK   += -O2
    LITEOS_COPTS_NODEBUG    += -O0
    LITEOS_CXXOPTS  += -O2
endif

ifeq ($(LOSCFG_SHELL), y)
    LITEOS_SCATTERLIB += -lshell
    LIB_SUBDIRS       += shell
    LITEOS_SHELL_INCLUDE  += -I $(LITEOSTOPDIR)/shell/include

    LITEOS_CMACRO     += -DLOSCFG_SHELL
endif

ifeq ($(LOSCFG_SHELL_EXCINFO), y)
    LITEOS_CMACRO     += -DLOSCFG_SHELL_EXCINFO
endif

ifeq ($(LOSCFG_PLATFORM_UART_WITHOUT_VFS), y)
    LITEOS_CMACRO += -DLOSCFG_PLATFORM_UART_WITHOUT_VFS
endif

ifeq ($(LOSCFG_NET_TELNET), y)
    LITEOS_SCATTERLIB += -ltelnet
    LIB_SUBDIRS       += net/telnet
    LITEOS_TELNET_INCLUDE   += \
        -I $(LITEOSTOPDIR)/net/telnet/include

    LITEOS_CMACRO     += -DLOSCFG_NET_TELNET
endif

ifeq ($(LOSCFG_PLATFORM_OSAPPINIT), y)
    LITEOS_CMACRO    += -DLOSCFG_PLATFORM_OSAPPINIT
else ifeq ($(LOSCFG_TEST), y)
    LITEOS_CMACRO    += -DLOSCFG_TEST
    LITEOS_MINISYS   := y
    -include $(LITEOSTOPDIR)/test/test.mk
endif

ifeq ($(LOSCFG_VENDOR), y)
    LITEOS_CMACRO     += -DLOSCFG_VENDOR
endif

ifeq ($(LOSCFG_TOOLS_IPERF), y)
    LITEOS_BASELIB    += -liperf
    LIB_SUBDIRS       += tools/iperf-2.0.5

    LITEOS_CMACRO     += -DLOSCFG_TOOLS_IPERF
endif

ifeq ($(LOSCFG_MEMORY_CHECK), y)
    ifeq ($(LOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK), 0)
        LITEOS_CMACRO += -DLOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK=0
    else
        LITEOS_CMACRO += -DLOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK=1
    endif
    ifeq ($(LOSCFG_BASE_MEM_NODE_SIZE_CHECK), 1)
        LITEOS_CMACRO += -DLOSCFG_BASE_MEM_NODE_SIZE_CHECK=1
    else
        LITEOS_CMACRO += -DLOSCFG_BASE_MEM_NODE_SIZE_CHECK=0
    endif
else
    LITEOS_CMACRO += -DLOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK=0
    LITEOS_CMACRO += -DLOSCFG_BASE_MEM_NODE_SIZE_CHECK=0
endif

############################# Tools && Debug Option End #################################

############################# Security Option Begin ##############################
LITEOS_SSP = -fno-stack-protector
ifeq ($(LOSCFG_CC_STACKPROTECTOR), y)
    LITEOS_SSP = -fstack-protector --param ssp-buffer-size=4
    LITEOS_CMACRO += -DLOSCFG_CC_STACKPROTECTOR
endif

ifeq ($(LOSCFG_CC_STACKPROTECTOR_ALL), y)
    LITEOS_SSP = -fstack-protector-all
    LITEOS_CMACRO += -DLOSCFG_CC_STACKPROTECTOR_ALL
endif

############################# Security Option End ##############################
############################# Platform Option Begin#################################

    -include $(LITEOSTOPDIR)/platform/bsp/$(LITEOS_PLATFORM)/board.mk



    LITEOS_BASELIB +=  -lbspcommon
    LIB_SUBDIRS    += platform/bsp/common
############################# Platform Option End #################################

LITEOS_CXXINCLUDE += \
    $(LITEOS_LIBC_INCLUDE) \
    -I $(LITEOSTOPDIR)/lib/cxxstl/c++/$(VERSION_NUM) \
    -I $(LITEOSTOPDIR)/lib/cxxstl/gccinclude \
    -I $(LITEOSTOPDIR)/lib/cxxstl/gdbinclude \
    -I $(LITEOSTOPDIR)/lib/cxxstl/c++/$(VERSION_NUM)/ext \
    -I $(LITEOSTOPDIR)/lib/cxxstl/c++/$(VERSION_NUM)/backward \
    -I $(LITEOSTOPDIR)/compat/posix/include \
    -I $(LITEOSTOPDIR)/lib/libm/include \
    -I $(LITEOSTOPDIR)/lib/libc/include \
    -I $(LITEOSTOPDIR)/fs/include \
    -I $(LITEOSTOPDIR)/kernel/include \

ifeq ($(LOSCFG_COMPILER_HISIV600), y)
    LITEOS_CXXINCLUDE += \
        -I $(LITEOSTOPDIR)/lib/cxxstl/c++/$(VERSION_NUM)/arm-$(COMPILE_NAME)-linux-gnueabi
else ifeq ($(LOSCFG_COMPILER_HUAWEILITEOS), y)
    LITEOS_CXXINCLUDE += \
		-I $(LITEOSTOPDIR)/lib/cxxstl/c++/$(VERSION_NUM)/arm-linux-
else
    LITEOS_CXXINCLUDE += \
        -I $(LITEOSTOPDIR)/lib/cxxstl/c++/$(VERSION_NUM)/arm-$(COMPILE_NAME)-linux-uclibcgnueabi
endif
LITEOS_CXXOPTS_BASE  += -std=c++11 -nostdlib -nostdinc -nostdinc++ -fexceptions -fpermissive -fno-use-cxa-atexit -fno-builtin -frtti




LITEOS_EXTKERNEL_INCLUDE   := $(LITEOS_CPPSUPPORT_INCLUDE) $(LITEOS_DYNLOAD_INCLUDE) \
                              $(LITEOS_RUNSTOP_INCLUDE)    $(LITEOS_SCATTER_INCLUDE)
LITEOS_COMPAT_INCLUDE      := $(LITEOS_CMSIS_INCLUDE)      $(LITEOS_POSIX_INCLUDE) \
                              $(LITEOS_LINUX_INCLUDE)
LITEOS_FS_INCLUDE          := $(LITEOS_VFS_INCLUDE)        $(LITEOS_FAT_CACHE_INCLUDE) \
                              $(LITEOS_VFS_MTD_INCLUDE)    $(LITEOS_PROC_INCLUDE)
LITEOS_NET_INCLUDE         := $(LITEOS_LWIP_SACK_INCLUDE)
LITEOS_LIB_INCLUDE         := $(LITEOS_LIBC_INCLUDE)       $(LITEOS_LIBM_INCLUDE) \
                              $(LITEOS_ZLIB_INCLUDE)
LITEOS_DRIVERS_INCLUDE     := $(LITEOS_CELLWISE_INCLUDE)   $(LITEOS_GPIO_INCLUDE) \
                              $(LITEOS_HIDMAC_INCLUDE)     $(LITEOS_HIETH_SF_INCLUDE) \
                              $(LITEOS_HIGMAC_INCLUDE)     $(LITEOS_I2C_INCLUDE) \
                              $(LITEOS_LCD_INCLUDE)        $(LITEOS_MMC_INCLUDE) \
                              $(LITEOS_MTD_NAND_INCLUDE)   $(LITEOS_MTD_SPI_NOR_INCLUDE) \
                              $(LITEOS_RANDOM_INCLUDE)     $(LITEOS_RTC_INCLUDE) \
                              $(LITEOS_SPI_INCLUDE)        $(LITEOS_USB_INCLUDE) \
                              $(LITEOS_WTDG_INCLUDE)
LOSCFG_TOOLS_DEBUG_INCLUDE := $(LITEOS_SHELL_INCLUDE)      $(LITEOS_UART_INCLUDE) \
                              $(LITEOS_TELNET_INCLUDE)


FP = -fno-omit-frame-pointer
LITEOS_COPTS_BASE  := -fno-aggressive-loop-optimizations -fno-builtin -nostdinc -nostdlib -mno-unaligned-access $(WARNING_AS_ERROR) $(LITEOS_SSP)
LITEOS_COPTS_EXTRA := -Wnonnull  -std=c99 -Wpointer-arith -Wstrict-prototypes \
                      -Wno-write-strings -mthumb-interwork -ffunction-sections \
                      -fdata-sections -fno-exceptions -fno-short-enums $(FP)
ifeq ($(LOSCFG_THUMB), y)
LITEOS_COPTS_EXTRA_INTERWORK := $(LITEOS_COPTS_EXTRA) -mthumb
LITEOS_CMACRO     += -DLOSCFG_INTERWORK_THUMB
else
LITEOS_COPTS_EXTRA_INTERWORK := $(LITEOS_COPTS_EXTRA)
#-fno-inline
endif

ifeq ($(LOSCFG_LLTREPORT) ,y)
LITEOS_GCOV_OPTS := -fprofile-arcs -ftest-coverage
LITEOS_BASELIB += -lgcov
LITEOS_CMACRO  += -DLOSCFG_LLTREPORT
endif
LITEOS_LD_OPTS := -nostartfiles -static --gc-sections
LITEOS_LD_OPTS += $(LITEOS_DYNLOADOPTS)
LITEOS_LD_PATH += -L$(LITEOS_SCRIPTPATH)/ld -L$(LITEOSTOPDIR)/platform/bsp/$(LITEOS_PLATFORM) -L$(OUT)/lib -L$(LITEOS_LIB_BIGODIR)
ifeq ($(LOSCFG_VENDOR) ,y)
LITEOS_LD_PATH +=  -L$(OUT)/lib/rdk -L$(OUT)/lib/sdk \
                   -L$(OUT)/lib/main_server
endif

ifeq ($(LOSCFG_KERNEL_CPP_EXCEPTIONS_SUPPORT),y)
    LITEOS_LD_SCRIPT := -T$(LITEOSTOPDIR)/liteosexc.ld
else
    LITEOS_LD_SCRIPT := -T$(LITEOSTOPDIR)/liteos.ld
endif


# temporary
LITEOS_INCLUDE += \
        -I $(LITEOSTOPDIR)/kernel/base/include
LITEOS_CXXINCLUDE += \
        -I $(LITEOSTOPDIR)/kernel/base/include
