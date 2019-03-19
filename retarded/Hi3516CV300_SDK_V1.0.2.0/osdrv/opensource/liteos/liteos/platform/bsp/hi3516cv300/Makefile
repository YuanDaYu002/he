include $(LITEOSTOPDIR)/config.mk

ARFLAGS = cr
LIBOUT = $(BUILD)/platform/bsp/hi3516cv300
LIB = $(OUT)/lib/libhi3516cv300.a

RM = -rm -rf

NET_SRC = net
USB_SRC = usb
MMC_SRC = mmc
EXTCMD_SRC = extcmd
DVFS_SRC = dvfs
RANDOM_SRC = random

SRCS = $(wildcard *.c) $(wildcard config/*.c) $(wildcard mmu/*.c)

ifeq ($(LOSCFG_PLATFORM_DVFS), y)
SRCS += $(wildcard dvfs/*.c)
endif

ifeq ($(LOSCFG_DRIVERS_NETDEV), y)
SRCS += $(wildcard $(NET_SRC)/*.c)
endif

ifeq ($(LOSCFG_PLATFORM_ADAPT), y)
SRCS += $(wildcard os_adapt/*.c)
endif

ifeq ($(LOSCFG_DRIVERS_USB), y)
SRCS += $(wildcard $(USB_SRC)/*.c)
endif
ifeq ($(LOSCFG_DRIVERS_MMC), y)
SRCS += $(wildcard $(MMC_SRC)/*.c)
MMC_INCLUDE += -I $(LITEOSTOPDIR)/drivers/mmc/host/$(MMC_HOST_DIR)
LITEOS_CFLAGS += $(MMC_INCLUDE)
LITEOS_CFLAGS_NODEBUG += $(MMC_INCLUDE)
endif

ifeq ($(LOSCFG_DRIVERS_RANDOM), y)
SRCS += $(wildcard $(RANDOM_SRC)/*.c)
endif
ifeq ($(LOSCFG_SHELL), y)
SRCS += $(EXTCMD_SRC)/ddrs_shell.c \
		$(EXTCMD_SRC)/reset_shell.c	\
		$(EXTCMD_SRC)/misc_shell.c
endif

COBJS = $(patsubst %.c,$(LIBOUT)/%.o,$(SRCS))


ALL_ASSRCS = $(wildcard *.S)
ASSRCS = $(subst board.ld.S,,$(ALL_ASSRCS))

ASLIBOUTSRCS = $(patsubst %.S,$(LIBOUT)/%.s,$(ASSRCS))
ASOBJS = $(patsubst %.S,$(LIBOUT)/%.o,$(ASSRCS))


all: $(LIBOUT) $(LIB) $(COBJS) $(ASOBJS)

$(COBJS): $(LIBOUT)/%.o : %.c
ifeq ($(LOSCFG_THUMB), y)
	@$(CC) $(LITEOS_CFLAGS_NODEBUG) -c $< -o $@
else
	@$(CC) $(LITEOS_CFLAGS) -c $< -o $@
endif
$(ASLIBOUTSRCS): $(LIBOUT)/%.s : %.S
	@$(CC) -E $(LITEOS_CFLAGS) $< -o $@

$(ASOBJS): $(LIBOUT)/%.o : $(LIBOUT)/%.s
	@$(AS) $(LITEOS_ASFLAGS) -c $< -o $@

OBJS = $(COBJS) $(ASOBJS)

$(OBJS): $(LIBOUT)
$(LIB): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $(OBJS)

$(LIBOUT):
	mkdir -p $(LIBOUT)
	mkdir -p $(LIBOUT)/config
	mkdir -p $(LIBOUT)/mmu
	mkdir -p $(LIBOUT)/dvfs
ifeq ($(LOSCFG_SHELL), y)
	mkdir -p $(LIBOUT)/$(EXTCMD_SRC)
endif
ifeq ($(LOSCFG_DRIVERS_NETDEV), y)
	mkdir -p $(LIBOUT)/$(NET_SRC)
endif
ifeq ($(LOSCFG_DRIVERS_USB), y)
	mkdir -p $(LIBOUT)/$(USB_SRC)
endif
ifeq ($(LOSCFG_DRIVERS_MMC), y)
	mkdir -p $(LIBOUT)/$(MMC_SRC)
endif
ifeq ($(LOSCFG_DRIVERS_RANDOM), y)
	mkdir -p $(LIBOUT)/$(RANDOM_SRC)
endif
ifeq ($(LOSCFG_PLATFORM_ADAPT), y)
	mkdir -p $(LIBOUT)/os_adapt
endif

clean:
	@$(RM) $(OBJS) $(LIB) $(LIBOUT) *.bak *~

.PHONY: all clean
