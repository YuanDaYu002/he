cur_dir:=$(PWD)



LITEOSTOPDIR = $(cur_dir)/../..

export LITEOSTOPDIR

SAMPLE_TOP_ROOT = $(cur_dir)

SAMPLE_OUT_ROOT = $(SAMPLE_TOP_ROOT)/out
SAMPLE_OUT_LIB_ROOT = $(SAMPLE_TOP_ROOT)/out/lib
SAMPLE_OUT_OBJ_ROOT = $(SAMPLE_TOP_ROOT)/out/obj

SAMPLE_SCRIPT_ROOT = $(SAMPLE_TOP_ROOT)/script
SAMPLE_SRC_ROOT = $(SAMPLE_TOP_ROOT)/src

include $(LITEOSTOPDIR)/config.mk

RM = -rm -rf



SRCS := 

SRCS += $(SAMPLE_SRC_ROOT)/common.c


SRCS += $(SAMPLE_SRC_ROOT)/sample_scatter.c


SRCS := $(notdir $(SRCS))



OBJS = $(patsubst %.c,$(SAMPLE_OUT_OBJ_ROOT)/%.o,$(SRCS))


.PHONY: all clean liteos_image prepare  liteos_image_clean


include $(SAMPLE_TOP_ROOT)/cfg.mak
include $(SAMPLE_TOP_ROOT)/script/config_to_macro.mak

DEFAULT:all

TARGET = sample


CreateDir        =   $(shell [ -d $1 ] || mkdir -p $1 || echo ":mkdir '$1' fail")
RemoveDir        =   $(shell [ -d $1 ] && rm -rf $1 && echo -e "rmdir '$1'\t [ OK ]" || echo ":rm dir '$1' fail")

TMP_DIR := $(cur_dir)/out/tmp/
OBJ_DIR := $(cur_dir)/out/lib/OBJ



dummy :=  $(call CreateDir, $(SAMPLE_OUT_ROOT))
dummy :=  $(call CreateDir, $(SAMPLE_OUT_ROOT)/bin)
dummy :=  $(call CreateDir, $(SAMPLE_OUT_ROOT)/burn)
dummy :=  $(call CreateDir, $(SAMPLE_OUT_LIB_ROOT))
dummy :=  $(call CreateDir, $(SAMPLE_OUT_OBJ_ROOT))
dummy :=  $(call CreateDir, $(SAMPLE_OUT_LIB_ROOT))

dummy :=  $(call CreateDir, $(TMP_DIR))
dummy :=  $(call CreateDir, $(OBJ_DIR))


prepare:
	@echo "Attention!!! Please make a choice:" 
	@if [[ "$(CFG_SCATTER_FLAG)" != "yes"  ]];then \
		echo "	1. If you want to use scatter, you should use \"make liteos_image \" instead of \"make\".Now press Ctrl+C to finish make";\
		echo "	2. If you just want to make an regular image, then just press enter to continue";\
		read tmp;\
	fi
	
all: prepare $(OBJS)  $(TARGET)
	@echo "--------------make finish-------------" 

clean:
	@$(RM)  $(SAMPLE_OUT_ROOT)

VSS_INCLUDE := 



top_srcs = $(SAMPLE_SRC_ROOT)/sample_scatter.c
top_srcs += $(SAMPLE_SRC_ROOT)/common.c




top_objs = $(patsubst %.c,$(SAMPLE_OUT_OBJ_ROOT)/%.o, $(notdir $(top_srcs)))


	
$(top_objs):$(SAMPLE_OUT_OBJ_ROOT)/%.o:$(SAMPLE_SRC_ROOT)/%.c
	$(CC) $(LITEOS_CFLAGS) $(VSS_DEFS) $(VSS_INCLUDE) -c $< -o $@
	

LITEOS_LIBS += -lsample 


SAMPLELIB = $(SAMPLE_TOP_ROOT)/out/libsample.a

$(SAMPLELIB):$(OBJS)
	@$(AR) -rcs $@ $(OBJS) 
	
	
lib_copy:
	# This step is very important. All the lib files should be copied to the 
	# $(SAMPLE_OUT_LIB_ROOT) .
	@cp -rf $(LITEOSTOPDIR)/out/$(LITEOS_PLATFORM)/lib/*.a     $(SAMPLE_OUT_LIB_ROOT)
	mv $(SAMPLE_TOP_ROOT)/out/libsample.a  $(SAMPLE_OUT_LIB_ROOT)/libsample.a
	
	# Call the scatter.sh script.
	if [[ "$(CFG_SCATTER_FLAG)" == "yes" && "$(CFG_FAST_IMAGE)" != "yes"  ]];then \
	$(LITEOSTOPDIR)/tools/scripts/scatter_sr/scatter.sh  $(CROSS_COMPILE) n    y    $(LITEOSTOPDIR)/tools/scripts/scatter_sr \
		$(LITEOSTOPDIR)/tools/scripts/ld     $(TMP_DIR)    $(OBJ_DIR)  ;\
	else \
		cp $(SAMPLE_TOP_ROOT)/script/wow_orignal.ld   $(LITEOSTOPDIR)/tools/scripts/ld/wow.ld;\
		cp $(SAMPLE_TOP_ROOT)/script/scatter_orignal.ld   $(LITEOSTOPDIR)/tools/scripts/ld/scatter.ld;\
	fi

	
LITEOS_LIBDEP := --start-group   $(LITEOS_LIBS)   --end-group

#$(error ------------ $(LITEOS_LDFLAGS))
MY_LD_FLAGS = -nostartfiles -static  -L$(LITEOSTOPDIR)/platform/bsp/$(LITEOS_PLATFORM) \
		-L$(LITEOSTOPDIR)/tools/scripts/ld -T$(LITEOSTOPDIR)/liteos.ld $(LITEOS_TABLES_LDFLAGS)
MY_LD_FLAGS += -L$(SAMPLE_OUT_LIB_ROOT)/OBJ   -L$(SAMPLE_OUT_LIB_ROOT)

$(TARGET):  $(SAMPLELIB) lib_copy
	$(LD) $(MY_LD_FLAGS) --gc-sections -Map=$(SAMPLE_OUT_ROOT)/bin/sample.map -o $(SAMPLE_OUT_ROOT)/bin/sample   $(LITEOS_LIBDEP)
	$(OBJCOPY) -O binary $(SAMPLE_OUT_ROOT)/bin/sample $(SAMPLE_OUT_ROOT)/burn/sample.bin
	$(OBJDUMP) -d $(SAMPLE_OUT_ROOT)/bin/sample >$(SAMPLE_OUT_ROOT)/bin/sample.asm
	chmod u+x $(LITEOSTOPDIR)/tools/scripts/scatter_sr/scatter_size.sh 
	$(LITEOSTOPDIR)/tools/scripts/scatter_sr/scatter_size.sh    $(SAMPLE_OUT_ROOT)/bin/sample
	

liteos_image:
	@chmod u+x $(SAMPLE_TOP_ROOT)/script/gen_image.sh
	@$(SAMPLE_TOP_ROOT)/script/gen_image.sh  $(LITEOS_PLATFORM)

liteos_image_clean: clean
	@cp $(SAMPLE_SCRIPT_ROOT)/wow_orignal.ld    $(LITEOSTOPDIR)/tools/scripts/ld/wow.ld   -f
	@cp $(SAMPLE_SCRIPT_ROOT)/scatter_orignal.ld     $(LITEOSTOPDIR)/tools/scripts/ld/scatter.ld  -f	
