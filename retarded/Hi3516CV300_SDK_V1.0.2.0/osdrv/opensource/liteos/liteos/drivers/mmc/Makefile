include $(LITEOSTOPDIR)/config.mk

ARFLAGS = cr
LIBOUT = $(BUILD)/drivers/mmc
LIBDRVMMC = $(OUT)/lib/libmmc.a
LIB = $(LIBDRVMMC)

RM = -rm -rf

HOST_SRCS = $(wildcard host/$(MMC_HOST_DIR)/*.c)
SRCS = $(wildcard core/*.c $(HOST_SRCS))
COBJS = $(patsubst %.c,$(LIBOUT)/%.o,$(SRCS))

MMC_INCLUDE += -I $(LITEOSTOPDIR)/drivers/mmc/core
MMC_INCLUDE += -I $(LITEOSTOPDIR)/drivers/mmc/host/$(MMC_HOST_DIR)
LITEOS_CFLAGS += $(MMC_INCLUDE)
all: $(LIBOUT) $(LIBDRVMMC)

$(COBJS): $(LIBOUT)/%.o : %.c
	@$(CC) $(LITEOS_CFLAGS) -c $< -o $@

OBJS = $(COBJS)

$(OBJS): $(LIBOUT)

$(LIBDRVMMC): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $(OBJS)

$(LIBOUT):
	mkdir -p $(LIBOUT)/
	mkdir -p $(LIBOUT)/core
	mkdir -p $(LIBOUT)/host/$(MMC_HOST_DIR)
clean:
	@$(RM) $(OBJS) $(LIB) $(LIBOUT) *.bak *~

.PHONY: all clean
