
WOW_SRC := $(LITEOSTOPDIR)/platform/bsp/$(LITEOS_PLATFORM)/os_adapt/os_adapt.c
SCATTER_SRC := $(LITEOSTOPDIR)/platform/bsp/$(LITEOS_PLATFORM)/os_adapt/os_adapt.c

############## make wow, when only runstop used #####################
wow_image: $(__LIBS)
ifeq ($(LOSCFG_KERNEL_RUNSTOP), y)
	@echo "LITEOS_IMAGE_MACRO := -DMAKE_WOW_IMAGE" > $(SCRIPTS_PATH)/scatter_sr/image.mk
	@$(CC) -E $(LITEOSTOPDIR)/platform/bsp/$(LITEOS_PLATFORM)/board.ld.S -o $(LITEOSTOPDIR)/platform/bsp/$(LITEOS_PLATFORM)/board.ld -P
	touch $(WOW_SRC)

	for dir in $(LITEOS_SUBDIRS); \
		do $(MAKE) -C $$dir all || exit 1; \
	done
	$(SCRIPTS_PATH)/scatter_sr/clear_ld.sh $(SCRIPTS_PATH)/ld

	$(LD) $(LITEOS_LDFLAGS) $(LITEOS_TABLES_WOW_LDFLAGS) -Map=$(BUILD)/$(LITEOS_TARGET).map -o $(BUILD)/$(LITEOS_TARGET) --start-group $(LITEOS_LIBDEP) --end-group
endif

wow: wow_image
ifeq ($(LOSCFG_KERNEL_RUNSTOP), y)
	@echo "WOW_SWITCH := y" > $(SCRIPTS_PATH)/scatter_sr/switch.mk
	@echo "SCATTER_SWITCH := n" >> $(SCRIPTS_PATH)/scatter_sr/switch.mk
	$(SCRIPTS_PATH)/scatter_sr/liblist.sh wow $(BUILD)/$(LITEOS_TARGET) $(BUILD)/$(LITEOS_TARGET).map $(SCRIPTS_PATH)/scatter_sr $(OUT)/lib
endif

################ make scatter, when only scatter used ##############
scatter_image: $(__LIBS)
ifeq ($(LOSCFG_KERNEL_SCATTER), y)
	@echo "LITEOS_IMAGE_MACRO := -DMAKE_SCATTER_IMAGE" > $(SCRIPTS_PATH)/scatter_sr/image.mk
	@$(CC) -E $(LITEOSTOPDIR)/platform/bsp/$(LITEOS_PLATFORM)/board.ld.S -o $(LITEOSTOPDIR)/platform/bsp/$(LITEOS_PLATFORM)/board.ld -P
	touch $(SCATTER_SRC)

	for dir in $(LITEOS_SUBDIRS); \
		do $(MAKE) -C $$dir all || exit 1; \
	done
	$(SCRIPTS_PATH)/scatter_sr/clear_ld.sh $(SCRIPTS_PATH)/ld

	$(LD) $(LITEOS_LDFLAGS) $(LITEOS_TABLES_WOW_LDFLAGS) $(LITEOS_TABLES_SCATTER_LDFLAGS) -Map=$(BUILD)/$(LITEOS_TARGET).map -o $(BUILD)/$(LITEOS_TARGET) --start-group $(LITEOS_LIBDEP) --end-group
endif

scatter: scatter_image
ifeq ($(LOSCFG_KERNEL_SCATTER), y)
	@echo "WOW_SWITCH := n" > $(SCRIPTS_PATH)/scatter_sr/switch.mk
	@echo "SCATTER_SWITCH := y" >> $(SCRIPTS_PATH)/scatter_sr/switch.mk
	$(SCRIPTS_PATH)/scatter_sr/liblist.sh scatter $(BUILD)/$(LITEOS_TARGET) $(BUILD)/$(LITEOS_TARGET).map $(SCRIPTS_PATH)/scatter_sr $(OUT)/lib
endif

################## make wow_scatter, when runstop and scatter both used ###################
wow_scatter: wow scatter
ifeq ($(LOSCFG_KERNEL_RUNSTOP), y)
ifeq ($(LOSCFG_KERNEL_SCATTER), y)
	@echo "WOW_SWITCH := y" > $(SCRIPTS_PATH)/scatter_sr/switch.mk
	@echo "SCATTER_SWITCH := y" >> $(SCRIPTS_PATH)/scatter_sr/switch.mk
	$(SCRIPTS_PATH)/scatter_sr/liblist.sh wow_scatter $(BUILD)/$(LITEOS_TARGET) $(BUILD)/$(LITEOS_TARGET).map $(SCRIPTS_PATH)/scatter_sr $(OUT)/lib
endif
endif

wow_scatter_clean:
	@-$(RM) $(SCRIPTS_PATH)/scatter_sr/image.mk

.PHONY: image wow_image scatter_image wow scatter wow_scatter wow_scatter_clean
