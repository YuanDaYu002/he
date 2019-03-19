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

LITEOS_LIBS_TARGET = libs_target

all: $(OUT) $(BUILD) $(LITEOS_LIBS_TARGET)

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

$(BUILD):
	mkdir -p $(BUILD)

$(LITEOS_LIBS_TARGET): $(CXX_INCLUDE) $(__LIBS)
	$(CC) -E $(LITEOSTOPDIR)/platform/bsp/$(LITEOS_PLATFORM)/board.ld.S -o $(LITEOSTOPDIR)/platform/bsp/$(LITEOS_PLATFORM)/board.ld -P
	for dir in $(LITEOS_SUBDIRS); \
		do $(MAKE) -C $$dir all || exit 1; \
	done
	date > $(OUT)/libsbuild.date
	@echo "=============== make lib done  ==============="

##### make menuconfig #####
menuconfig:
	make -C tools/menuconfig menuconfig

##### make #####


clean:cxxclean cclean
	@for dir in $(LITEOS_SUBDIRS); \
		do make -C $$dir clean|| exit 1; \
	done
	@$(RM) $(__OBJS) $(LITEOS_TARGET) $(OUT) $(BUILD) *.bak *~
	$(SCRIPTS_PATH)/scatter_sr/clear_ld.sh $(SCRIPTS_PATH)/ld
	@find platform/bsp/$(LITEOS_PLATFORM)/ -name board.ld -exec rm -rf {} \;
	@echo "clean $(LITEOS_PLATFORM) finish"

cleanall:clean
	@$(RM) -rf $(LITEOSTOPDIR)/out
	@find platform/bsp/ -name board.ld -exec rm -rf {} \;
	@cd sample/sample_osdrv;make clean;cd ../..;
	@cd sample/sample_dynload;make clean;cd ../..;
	@cd sample/sample_scatter;make liteos_image_clean;cd ../..;
	@echo "clean all"

.PHONY: all lib clean cleanall $(LITEOS_TARGET)
