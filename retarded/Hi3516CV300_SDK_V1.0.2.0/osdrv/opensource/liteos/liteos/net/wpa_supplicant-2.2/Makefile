include $(LITEOSTOPDIR)/config.mk

ARFLAGS = cr

all:
	mkdir -p $(OUT)/lib/
	cp -rf $(LITEOS_PLATFORM)/*.a $(OUT)/lib


clean:
	rm -rf $(OUT)/lib/*.a

.PHONY: all clean
