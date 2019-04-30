#
# image should be loaded at 0x01000000
#
TEXT_BASE = 0x81800000

PLATFORM_CPPFLAGS += -march=armv5te -mno-unaligned-access -DTEXT_BASE=$(TEXT_BASE)
