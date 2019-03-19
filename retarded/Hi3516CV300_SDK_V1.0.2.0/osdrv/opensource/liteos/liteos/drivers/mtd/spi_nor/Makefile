include $(LITEOSTOPDIR)/config.mk

ifeq ($(NAND_DRIVER_DIR),hisfc350)
SRCS_COMMON_DEL0 = src/common/hifmc_comomn.c
endif

ARFLAGS = cr
LIBOUT = $(BUILD)/drivers/mtd/spi_nor
LIBDRVSPI_NOR = $(OUT)/lib/libspinor_flash.a
LIB = $(LIBDRVSPI_NOR)

RM = -rm -rf

SRCS_HOST = $(wildcard src/$(NOR_DRIVER_DIR)/*.c)
COBJS_HOST = $(patsubst %.c,$(LIBOUT)/%.o,$(SRCS_HOST))

SRCS_COMMON_TMP0 = $(wildcard src/common/*.c)
SRCS_COMMON = $(subst $(SRCS_COMMON_DEL0),,$(SRCS_COMMON_TMP0))
COBJS_COMMON = $(patsubst %.c,$(LIBOUT)/%.o,$(SRCS_COMMON))

SPI_NOR_INCLUDE = -I $(LITEOSTOPDIR)/drivers/mtd/common/include/
SPI_NOR_INCLUDE += -I $(LITEOSTOPDIR)/drivers/mtd/spi_nor/src/common/
LITEOS_CFLAGS += $(SPI_NOR_INCLUDE)

all: $(LIBOUT) $(LIBDRVSPI_NOR)

$(COBJS_HOST): $(LIBOUT)/%.o : %.c
	@$(CC) $(LITEOS_CFLAGS) -c $< -o $@

OBJS_HOST = $(COBJS_HOST)

OBJS_HOST: $(LIBOUT)

$(COBJS_COMMON): $(LIBOUT)/%.o : %.c
	@$(CC) $(LITEOS_CFLAGS) -c $< -o $@

OBJS_COMMON = $(COBJS_COMMON)

OBJS_COMMON: $(LIBOUT)

$(LIBDRVSPI_NOR): $(OBJS_HOST) $(OBJS_COMMON)
	@$(AR) $(ARFLAGS) $@ $(OBJS_HOST) $(OBJS_COMMON)

$(LIBOUT):
	mkdir -p $(LIBOUT)/src
	mkdir -p $(LIBOUT)/src/common
	mkdir -p $(LIBOUT)/src/$(NOR_DRIVER_DIR)
clean:
	@$(RM) $(OBJS_HOST) $(LIB) $(LIBOUT) *.bak *~

.PHONY: all clean
