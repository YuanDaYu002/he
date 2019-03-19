include $(LITEOSTOPDIR)/config.mk

LIBOUT = $(BUILD)/os_adapt

RM = -rm -rf

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,$(LIBOUT)/%.o,$(SRCS))

all: $(LIBOUT) $(OBJS)

$(OBJS): $(LIBOUT)/%.o : %.c
	@$(CC) $(LITEOS_CFLAGS) -c $< -o $@

$(LIBOUT):
	mkdir -p $(LIBOUT)

clean:
	@$(RM) $(OBJS) $(LIBOUT) *.bak *~

.PHONY: all clean
