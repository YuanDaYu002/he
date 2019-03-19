include $(LITEOSTOPDIR)/config.mk

ARFLAGS = cr
LIBOUT = $(BUILD)/drivers/mtd/common
LIBMTD_COMMON = $(OUT)/lib/libmtd_common.a
LIB = $(LIBMTD_COMMON)

RM = -rm -rf

SRCS = $(wildcard src/*.c)
COBJS = $(patsubst %.c,$(LIBOUT)/%.o,$(SRCS))
MTD_COMMON_INCLUCE = $(LITEOSTOPDIR)/drivers/mtd/common/include
LITEOS_CFLAGS += -I $(MTD_COMMON_INCLUCE)

all: $(LIBOUT) $(LIBMTD_COMMON)

$(COBJS): $(LIBOUT)/%.o : %.c
	@$(CC) $(LITEOS_CFLAGS) -c $< -o $@

OBJS = $(COBJS)

$(OBJS):$(LIBOUT)

$(LIBMTD_COMMON): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $(OBJS)

$(LIBOUT):
	mkdir -p $(LIBOUT)/src

clean:
	@$(RM) $(OBJS) $(LIB) $(LIBOUT) *.bak *~

.PHONY: all clean
