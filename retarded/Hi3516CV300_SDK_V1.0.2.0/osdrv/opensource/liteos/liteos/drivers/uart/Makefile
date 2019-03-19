include $(LITEOSTOPDIR)/config.mk

ARFLAGS = cr
LIBOUT = $(BUILD)/drivers/uart
LIB = $(OUT)/lib/libuart.a

RM = -rm -rf

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst %.c,$(LIBOUT)/%.o,$(SRCS))

UART_INCLUDE += -I $(LITEOSTOPDIR)/drivers/uart/src -I $(LITEOSTOPDIR)/lib/libc/stdio
all: $(LIBOUT) $(LIB)

$(OBJS): $(LIBOUT)/%.o : %.c
	@$(CC) $(LITEOS_CFLAGS) $(UART_INCLUDE) -c $< -o $@

$(LIB): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $(OBJS)

$(LIBOUT):
	mkdir -p $(LIBOUT)/src

clean:
	@$(RM) $(OBJS) $(LIB) $(LIBOUT) *.bak *~

.PHONY: all clean
