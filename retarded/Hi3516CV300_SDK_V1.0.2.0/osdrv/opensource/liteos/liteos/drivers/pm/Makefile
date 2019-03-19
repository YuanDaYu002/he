include $(LITEOSTOPDIR)/config.mk

ARFLAGS = cr
LIBOUT = $(BUILD)/drivers/pm
LIBDRVPM = $(OUT)/lib/libpm.a
LIB = $(LIBDRVPM)

RM = -rm -rf

PM_SRCS = $(wildcard pm/*.c)
PROC_SRCS = $(wildcard proc/*.c)
REGULATOR_SRCS = $(wildcard regulator/*.c)
DVFS_SRCS = $(wildcard dvfs/*.c)
AVS_SRCS = $(wildcard avs/*.c)
CPU_SRCS = $(wildcard domain_cpu/*.c)
MEDIA_SRCS = $(wildcard domain_media/*.c)

SRCS = $(PM_SRCS) $(PROC_SRCS) $(REGULATOR_SRCS) $(DVFS_SRCS) $(AVS_SRCS) $(CPU_SRCS) $(MEDIA_SRCS)

COBJS = $(patsubst %.c,$(LIBOUT)/%.o,$(SRCS))

PM_INC = -I $(LITEOSTOPDIR)/drivers/pm/pm
PROC_INC = -I $(LITEOSTOPDIR)/drivers/pm/proc
REGULATOR_INC = -I $(LITEOSTOPDIR)/drivers/pm/regulator
DVFS_INC = -I $(LITEOSTOPDIR)/drivers/pm/dvfs
AVS_INC = -I $(LITEOSTOPDIR)/drivers/pm/avs
CPU_INC = -I $(LITEOSTOPDIR)/drivers/pm/domain_cpu
MEDIA_INC = -I $(LITEOSTOPDIR)/drivers/pm/domain_media

CHIP_INC = -I $(LITEOSTOPDIR)/platform/bsp/$(LITEOS_PLATFORM)/pm/include

LITEOS_CFLAGS += $(PM_INC) $(PROC_INC) $(REGULATOR_INC) $(DVFS_INC) $(AVS_INC) $(CPU_INC) $(MEDIA_INC) $(CHIP_INC)

all: $(LIBOUT) $(LIBDRVPM)

$(COBJS): $(LIBOUT)/%.o : %.c
	@$(CC) $(LITEOS_CFLAGS) -c $< -o $@

OBJS = $(COBJS)

$(OBJS): $(LIBOUT)

$(LIBDRVPM): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $(OBJS)

$(LIBOUT):
	mkdir -p $(LIBOUT)/
	mkdir -p $(LIBOUT)/pm
	mkdir -p $(LIBOUT)/proc
	mkdir -p $(LIBOUT)/regulator
	mkdir -p $(LIBOUT)/dvfs
	mkdir -p $(LIBOUT)/avs
	mkdir -p $(LIBOUT)/domain_cpu
	mkdir -p $(LIBOUT)/domain_media
clean:
	@$(RM) $(OBJS) $(LIB) $(LIBOUT) *.bak *~

.PHONY: all clean
