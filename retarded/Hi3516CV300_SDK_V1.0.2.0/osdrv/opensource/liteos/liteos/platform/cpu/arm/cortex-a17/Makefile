include $(LITEOSTOPDIR)/config.mk

ARFLAGS = cr
LIBOUT = $(BUILD)/platform/cpu/arm/cortex-a17
LIB = $(OUT)/lib/libcortex-a17.a

RM = -rm -rf

LOCAL_INCLUDE := \
	-I $(LITEOSTOPDIR)/kernel/base/include \
	-I $(LITEOSTOPDIR)/platform/cpu/arm/cortex-a17/include \

CSRCS = $(wildcard src/*.c)
COBJS = $(patsubst %.c,$(LIBOUT)/%.o,$(CSRCS))

ASSRCS = $(wildcard src/*.s)
ASOBJS = $(patsubst %.s,$(LIBOUT)/%.o,$(ASSRCS))

all: $(LIBOUT) $(LIB)

$(COBJS): $(LIBOUT)/%.o : %.c
	@$(CC) $(LITEOS_CFLAGS) $(LOCAL_INCLUDE) $(LITEOS_GCOV_OPTS) -c $< -o $@

$(ASOBJS): $(LIBOUT)/%.o : %.s
	@$(AS) $(LITEOS_ASFLAGS) -c $< -o $@

OBJS = $(COBJS) $(ASOBJS)

$(OBJS): $(LIBOUT)

$(LIB): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $(OBJS)

$(LIBOUT):
	mkdir -p $(LIBOUT)/src

clean:
	@$(RM) $(LIBOUT)  $(LIB)  *.bak *~

.PHONY: all clean
