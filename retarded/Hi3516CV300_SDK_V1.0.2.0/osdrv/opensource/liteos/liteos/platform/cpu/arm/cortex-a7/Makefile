include $(LITEOSTOPDIR)/config.mk

ARFLAGS = cr
LIBOUT = $(BUILD)/platform/cpu/arm/cortex-a7
LIB = $(OUT)/lib/libcortex-a7.a

RM = -rm -rf

LOCAL_INCLUDE := \
	-I $(LITEOSTOPDIR)/kernel/base/include \
	-I $(LITEOSTOPDIR)/platform/cpu/arm/cortex-a7/include \

CSRCS = $(wildcard src/*.c)
COBJS = $(patsubst %.c,$(LIBOUT)/%.o,$(CSRCS))

ASSRCS = $(wildcard src/*.S)
ASOBJS = $(patsubst %.S,$(LIBOUT)/%.o,$(ASSRCS))

all: $(LIBOUT) $(LIB)

$(COBJS): $(LIBOUT)/%.o : %.c
	@$(CC) $(LITEOS_CFLAGS) $(LOCAL_INCLUDE) $(LITEOS_GCOV_OPTS) -c $< -o $@

$(ASOBJS): $(LIBOUT)/%.o : %.S
	@$(AS) $(LITEOS_ASFLAGS) -c $< -o $@

OBJS = $(COBJS) $(ASOBJS)

$(OBJS): $(LIBOUT)

$(LIB): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $(OBJS)

$(LIBOUT):
	mkdir -p $(LIBOUT)/src

clean:
	@$(RM) $(LIBOUT)  $(LIB)  $(CORTEX_A7_OBJS) *.bak *~

.PHONY: all clean
