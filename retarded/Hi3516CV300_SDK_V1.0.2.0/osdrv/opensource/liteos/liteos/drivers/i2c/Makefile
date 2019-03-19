include $(LITEOSTOPDIR)/config.mk

ARFLAGS = cr
LIBOUT = $(BUILD)/drivers/i2c
LIB = $(OUT)/lib/libi2c.a

RM = -rm -rf

SRCS = src/i2c.c src/i2c_dev.c src/i2c_shell.c
ifdef LOSCFG_HOST_TYPE_HIBVT
SRCS += src/i2c-hibvt.c
else
SRCS += src/i2c-hisilicon.c
endif
COBJS = $(patsubst %.c,$(LIBOUT)/%.o,$(SRCS))

all: $(LIBOUT) $(LIB)

ifeq ($(LOSCFG_DRIVERS_HIDMAC), y)
LOCAL_INCLUDE = -I $(LITEOSTOPDIR)/drivers/hidmac/src
else
LOCAL_INCLUDE =
endif
$(COBJS): $(LIBOUT)/%.o : %.c
	@$(CC) $(LITEOS_CFLAGS) $(LOCAL_INCLUDE) -c $< -o $@

OBJS = $(COBJS)

$(OBJS): $(LIBOUT)

$(LIB): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $(OBJS)

$(LIBOUT):
	mkdir -p $(LIBOUT)/src

clean:
	@$(RM) $(OBJS) $(LIB) $(LIBOUT) *.bak *~

.PHONY: all clean
