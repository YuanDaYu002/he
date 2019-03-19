LITEOS_CPU_OPTS      := -mcpu=arm926ej-s
LITEOS_COPTS         += $(LITEOS_CPU_OPTS)
LITEOS_COPTS_NODEBUG += $(LITEOS_CPU_OPTS)
LITEOS_ASOPTS        += $(LITEOS_CPU_OPTS)
LITEOS_CXXOPTS_BASE  += $(LITEOS_CPU_OPTS)

LITEOS_ARM_ARCH := -march=armv5te

PLATFORM_INCLUDE := -I $(LITEOSTOPDIR)/platform/bsp/hi3516cv300 \
	    	-I $(LITEOSTOPDIR)/platform/cpu/arm/include \
		-I $(LITEOSTOPDIR)/platform/bsp/hi3516cv300/config \
		-I $(LITEOSTOPDIR)/platform/bsp/hi3516cv300/mmu \
		-I $(LITEOSTOPDIR)/platform/bsp/hi3516cv300/include \
		-I $(LITEOSTOPDIR)/platform/bsp/common

LITEOS_SCATTERLIB += -larm926 -lhi3516cv300

LITEOS_INCLUDE += $(PLATFORM_INCLUDE)

LITEOS_CXXINCLUDE += $(PLATFORM_INCLUDE)

LIB_SUBDIRS += platform/cpu/arm/arm926 platform/bsp/hi3516cv300

ifeq ($(LOSCFG_VENDOR), y)
-include $(LITEOSTOPDIR)/vendor/vendor_hi3516cv300.mk
endif

LITEOS_GCCLIB :=armv5te_arm9_soft
