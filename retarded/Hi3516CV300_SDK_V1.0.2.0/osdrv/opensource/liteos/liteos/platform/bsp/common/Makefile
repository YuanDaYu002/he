include $(LITEOSTOPDIR)/config.mk

ARFLAGS = cr
LIBOUT = $(BUILD)/platform/bsp/common
LIB = $(OUT)/lib/libbspcommon.a

RM = -rm -rf

SRCS = $(wildcard *.c) $(wildcard $(LITEOS_CPU_TYPE)/*.c)
COBJS = $(patsubst %.c,$(LIBOUT)/%.o,$(SRCS))

all: $(LIBOUT) $(LIB)

$(COBJS): $(LIBOUT)/%.o : %.c
	@$(CC) $(LITEOS_CFLAGS) -c $< -o $@

OBJS = $(COBJS)

$(LIB): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $(OBJS)

$(LIBOUT):
	mkdir -p $(LIBOUT)
	mkdir -p $(LIBOUT)/$(LITEOS_CPU_TYPE)
clean:
	@$(RM) $(OBJS) $(LIB) $(LIBOUT) *.bak *~

TEOS_CPU_TYPE.PHONY: all clean
