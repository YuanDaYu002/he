include $(LITEOSTOPDIR)/config.mk

ARFLAGS = cr
LIBOUT = $(BUILD)/lib/libcmini
LIB = $(OUT)/lib/libcmini.a

RM = -rm -rf

SRCS = $(wildcard ./*.c)
COBJS = $(patsubst %.c,$(LIBOUT)/%.o,$(SRCS))

all: $(LIBOUT) $(LIB)

$(COBJS): $(LIBOUT)/%.o : %.c
	@$(CC) $(LITEOS_CFLAGS) -c $< -o $@

OBJS = $(COBJS)

$(OBJS): $(LIBOUT)

$(LIB): $(OBJS)
	@$(AR) $(ARFLAGS) $@ $(OBJS)

$(LIBOUT):
	mkdir -p $(LIBOUT)/

clean:
	@$(RM) $(OBJS) $(LIB) *.bak *~

.PHONY: all clean
