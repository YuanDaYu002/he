# A simple Makefile for lib(libxxx.a)
# By Late Lee(http://www.latelee.org)

#include ../config.mk
include $(LITEOSTOPDIR)/config.mk
LIBOUT = $(LITEOSTOPDIR)/tools/elfs
RM = -rm -rf

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))
SOS = $(patsubst %.c,%.so,$(SRCS))
LITEOS_CFLAGS += -I $(LITEOSTOPDIR)/kernel/base/include -I $(LITEOSTOPDIR)/kernel/extended/include
all: $(OBJS) $(SOS)

$(OBJS): %.o : %.c
	$(CC) $(LITEOS_CFLAGS) -mlong-calls -mthumb -c $< -o $@
	$(OBJDUMP) -d $@ >$@.asm
	$(OBJDUMP) -S $@ >$@.S

$(SOS): %.so : %.c
	$(CC) $(LITEOS_CFLAGS) -mlong-calls -mthumb  $< -fPIC -shared -o $@
	$(OBJDUMP) -d $@ >$@.asm
	$(OBJDUMP) -S $@ >$@.S

clean:
	@$(RM) $(OBJS) *.so *.bak *.hex *.asm *.S *.o *~

.PHONY: all clean
