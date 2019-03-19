VSS_DEFS := 



ifeq ($(CFG_SCATTER_FLAG),yes)
VSS_DEFS += -DCFG_SCATTER_FLAG
endif



ifeq ($(CFG_FAST_IMAGE),yes)
VSS_DEFS += -DCFG_FAST_IMAGE
endif





