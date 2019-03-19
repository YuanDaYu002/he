include $(LITEOSTOPDIR)/config.mk

ifeq ($(NAND_DRIVER_DIR),hinfc620)
SRCS_COMMON_DEL0 = src/common/spi_nand_ids.c
SRCS_COMMON_DEL1 = src/common/hifmc_common.c
endif
ifeq ($(NAND_DRIVER_DIR),hifmc100_nand)
SRCS_COMMON_DEL0 = src/common/spi_nand_ids.c
endif

ifeq ($(NAND_DRIVER_DIR),hisnfc100)
SRCS_COMMON_DEL0 = src/common/parallel_nand_ids.c
SRCS_COMMON_DEL1 = src/common/hifmc_common.c
endif
ifeq ($(NAND_DRIVER_DIR),hifmc100)
SRCS_COMMON_DEL0 = src/common/parallel_nand_ids.c
endif

ARFLAGS = cr
LIBOUT = $(BUILD)/drivers/mtd/nand
LIBDRVNAND_FLASH = $(OUT)/lib/libnand_flash.a
LIB = $(LIBDRVNAND_FLASH)

RM = -rm -rf

SRCS_HOST = $(wildcard src/$(NAND_DRIVER_DIR)/*.c)
COBJS_HOST = $(patsubst %.c,$(LIBOUT)/%.o,$(SRCS_HOST))

SRCS_COMMON_TMP0 = $(wildcard src/common/*.c)

SRCS_COMMON_TMP1 = $(subst $(SRCS_COMMON_DEL0),,$(SRCS_COMMON_TMP0))
SRCS_COMMON = $(subst $(SRCS_COMMON_DEL1),,$(SRCS_COMMON_TMP1))
COBJS_COMMON = $(patsubst %.c,$(LIBOUT)/%.o,$(SRCS_COMMON))

NAND_INCLUDE = -I $(LITEOSTOPDIR)/drivers/mtd/common/include/
NAND_INCLUDE += -I $(LITEOSTOPDIR)/drivers/mtd/nand/src/common/
LITEOS_CFLAGS += $(NAND_INCLUDE)

all: $(LIBOUT) $(LIBDRVNAND_FLASH)

$(COBJS_HOST): $(LIBOUT)/%.o : %.c
	@$(CC) $(LITEOS_CFLAGS) -c $< -o $@

OBJS_HOST = $(COBJS_HOST)

OBJS_HOST: $(LIBOUT)

$(COBJS_COMMON): $(LIBOUT)/%.o : %.c
	@$(CC) $(LITEOS_CFLAGS) -c $< -o $@

OBJS_COMMON = $(COBJS_COMMON)

OBJS_COMMON: $(LIBOUT)


$(LIBDRVNAND_FLASH): $(OBJS_HOST) $(OBJS_COMMON)
	@$(AR) $(ARFLAGS) $@ $(OBJS_HOST) $(OBJS_COMMON)

$(LIBOUT):
	mkdir -p $(LIBOUT)/src
	mkdir -p $(LIBOUT)/src/common
	mkdir -p $(LIBOUT)/src/$(NAND_DRIVER_DIR)
clean:
	@$(RM) $(OBJS_HOST) $(LIB) $(LIBOUT) *.bak *~

.PHONY: all clean
