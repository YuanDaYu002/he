#################################################################
#
# bugs:
#      1. 需要显式指定库位置、名称；
#      2. make 及 make clean处理得不好(对于库，要么删除再编译，要么无操作)；
##################################################################
LITEOSTOPDIR := $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)

export LITEOSTOPDIR

-include $(LITEOSTOPDIR)/config.mk

RM = rm -rf
MAKE = make
__LIBS = libs

LITEOS_TARGET = vs_server
LITEOS_LIBS_TARGET = libs_target


all: $(OUT) $(BUILD) $(LITEOS_TARGET)
lib: $(OUT) $(BUILD) $(LITEOS_LIBS_TARGET)

##### cxx include #####
-include $(LITEOS_MK_PATH)/cxx.mk

##### make wow && make scatter && make wow_scatter #####
-include $(LITEOS_MK_PATH)/wow_scatter.mk

##### make dynload #####
-include $(LITEOS_MK_PATH)/dynload.mk

#-----need move when make version-----#
##### make lib #####
$(__LIBS): $(OUT) $(CXX_INCLUDE)
	cp -rf lib/$(LITEOS_GCCLIB)/*.a $(OUT)/lib

$(OUT):
	mkdir -p $(OUT)/lib
	@$(CC) -E $(LITEOSTOPDIR)/platform/bsp/$(LITEOS_PLATFORM)/board.ld.S -o $(LITEOSTOPDIR)/platform/bsp/$(LITEOS_PLATFORM)/board.ld -P

$(BUILD):
	mkdir -p $(BUILD)

$(LITEOS_LIBS_TARGET): $(__LIBS)
	for dir in $(LIB_SUBDIRS); \
		do $(MAKE) -C $$dir all || exit 1; \
	done
	@echo "=============== make lib done  ==============="

##### make menuconfig #####
menuconfig:
	make -C tools/menuconfig menuconfig

##### make #####

$(LITEOS_TARGET): $(__LIBS)
	@-rm $(SCRIPTS_PATH)/scatter_sr/image.mk
	touch $(LOSCFG_ENTRY_SRC)
ifeq ($(LOSCFG_KERNEL_RUNSTOP), y)
	touch $(WOW_SRC)
endif
ifeq ($(LOSCFG_KERNEL_SCATTER), y)
	touch $(SCATTER_SRC)
endif

	for dir in $(LITEOS_SUBDIRS); \
	do $(MAKE) -C $$dir all || exit 1; \
	done

ifeq ($(LITEOS_MINISYS), n)
ifeq ($(LOSCFG_KERNEL_RUNSTOP), y)
	$(SCRIPTS_PATH)/scatter_sr/scatter.sh $(CROSS_COMPILE) $(WOW_SWITCH) $(SCATTER_SWITCH) $(SCRIPTS_PATH)/scatter_sr $(SCRIPTS_PATH)/ld $(LITEOSTOPDIR)/liteos_tmp $(LIB_BIGODIR)
else ifeq ($(LOSCFG_KERNEL_SCATTER), y)
	$(SCRIPTS_PATH)/scatter_sr/scatter.sh $(CROSS_COMPILE) $(WOW_SWITCH) $(SCATTER_SWITCH) $(SCRIPTS_PATH)/scatter_sr $(SCRIPTS_PATH)/ld $(LITEOSTOPDIR)/liteos_tmp $(LIB_BIGODIR)
endif
else
	$(SCRIPTS_PATH)/scatter_sr/clear_ld.sh $(SCRIPTS_PATH)/ld
endif

	$(LD) $(LITEOS_LDFLAGS) $(LITEOS_TABLES_WOW_LDFLAGS) $(LITEOS_TABLES_SCATTER_LDFLAGS) $(LITEOS_TABLES_LDFLAGS) $(LITEOS_DYNLDFLAGS) -Map=$(OUT)/$@.map -o $(OUT)/$@ --start-group $(LITEOS_LIBDEP) --end-group

	$(OBJCOPY) -O binary $(OUT)/$@ $(OUT)/$@.bin
	$(OBJDUMP) -d $(OUT)/$@ >$(OUT)/$@.asm
ifeq ($(LOSCFG_KERNEL_SCATTER), y)
	$(SCRIPTS_PATH)/scatter_sr/scatter_size.sh $(OUT)/$(LITEOS_TARGET)
endif

clean:
	@for dir in $(LITEOS_SUBDIRS); \
		do make -C $$dir clean|| exit 1; \
	done
	$(RM) $(__OBJS) $(LITEOS_TARGET) $(OUT) $(BUILD) *.bak *~
	$(SCRIPTS_PATH)/scatter_sr/clear_ld.sh $(SCRIPTS_PATH)/ld
	@find platform/bsp/$(LITEOS_PLATFORM)/ -name board.ld -exec rm -rf {} \;
	@echo "clean $(LITEOS_PLATFORM) finish"

cleanall:
	@rm -rf $(LITEOSTOPDIR)/out
	@find platform/bsp/ -name board.ld -exec rm -rf {} \;
	@cd sample/sample_osdrv;make clean;cd ../..;
	@echo "clean all"

.PHONY: all lib clean cleanall $(LITEOS_TARGET)
