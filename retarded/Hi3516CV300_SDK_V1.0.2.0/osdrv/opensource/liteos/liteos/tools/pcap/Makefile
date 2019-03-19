include $(LITEOSTOPDIR)/config.mk

MODULE_NAME := $(notdir $(shell pwd))

LIBOUT = $(BUILD)/$(MODULE_NAME)
LIB_PCAP = $(OUT)/lib/lib$(MODULE_NAME).a


CMNFLAGS:=-Wall -Wpointer-arith -Wundef -Wno-write-strings \
        -mno-thumb-interwork \
        $(LITEOS_ARM_ARCH) -O2 -ffunction-sections -fdata-sections \
        -fno-short-enums
CFLAGS += $(LITEOS_CMACRO) $(CMNFLAGS) -Wstrict-prototypes -fno-builtin

ARFLAGS = cr
RM = -rm -rf

LOCAL_INCLUDE := \
		-I $(LITEOSTOPDIR)/tools/pcap/missing \
		-I $(LITEOSTOPDIR)/net/lwip_sack/include/lwip \
		-I $(LITEOSTOPDIR)/net/lwip_sack/include/ipv4/lwip \
		-I $(LITEOSTOPDIR)/net/lwip_sack/include/lwip \
		-I $(LITEOSTOPDIR)/tools/pcap


LOCAL_SRCS = $(wildcard *.c)

LOCAL_FLAGS :=  $(LOCAL_INCLUDE) -Wno-error -DHAVE_CONFIG_H -U__linux__ -D_U_=

LIB_PCAP_COBJ =   $(patsubst %.c,$(LIBOUT)/%.o,$(LOCAL_SRCS))


LIB_PCAP_OBJ = $(LIB_PCAP_COBJ)

$(LIB_PCAP_OBJ): $(LIBOUT)

all:  $(LIBOUT) $(LIB_PCAP)

$(LIB_PCAP_COBJ): $(LIBOUT)/%.o : %.c
	@$(CC) $(CFLAGS) $(LITEOS_INCLUDE) $(LOCAL_FLAGS) -c $< -o $@

$(LIB_PCAP): $(LIB_PCAP_COBJ)
	@$(AR) $(ARFLAGS) $@ $(LIB_PCAP_COBJ)

$(LIBOUT):
	mkdir -p $(LIBOUT)

clean:
	@$(RM) $(LIBOUT)  $(LIB_PCAP) *.bak *~

.PHONY: all clean
