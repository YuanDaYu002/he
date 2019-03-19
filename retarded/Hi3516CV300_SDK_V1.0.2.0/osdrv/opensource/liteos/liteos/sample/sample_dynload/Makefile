LITEOSTOPDIR ?= ../..

SAMPLE_OUT = .

include $(LITEOSTOPDIR)/config.mk
RM = -rm -rf

LITEOS_LIBDEPS := --start-group $(LITEOS_LIBDEP) --end-group

SRCS = $(wildcard sample.c)

OBJS = $(patsubst %.c,$(SAMPLE_OUT)/%.o,$(SRCS))

all: $(OBJS)

clean:
	@$(RM) *.o  sample *.bin *.map *.asm *.so
	make -C so clean
	cd $(LITEOSTOPDIR);make clean;cd -

$(OBJS): $(SAMPLE_OUT)/%.o : %.c
	$(CC) $(LITEOS_CFLAGS) -c $< -o $@
	$(LD) $(LITEOS_LDFLAGS)  $(LITEOS_DYNLDFLAGS) --gc-sections -Map=$(SAMPLE_OUT)/sample.map -o $(SAMPLE_OUT)/sample ./$@ $(LITEOS_LIBDEPS)
	$(OBJCOPY) -O binary $(SAMPLE_OUT)/sample $(SAMPLE_OUT)/sample.bin
	$(OBJDUMP) -d $(SAMPLE_OUT)/sample >$(SAMPLE_OUT)/sample.asm

prepare:
	make -C so all
	chmod +x test.sh
	./test.sh
	cd $(LITEOSTOPDIR);make;cd -
