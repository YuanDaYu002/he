
CXX_INCLUDE := cxx_include

$(CXX_INCLUDE):
ifeq ($(LOSCFG_KERNEL_CPPSUPPORT), y)
	$(CXX_PATH)/cxx_copy.sh $(CROSS_COMPILE) $(VERSION_NUM) CXX
else
	$(CXX_PATH)/cxx_copy.sh $(CROSS_COMPILE) $(VERSION_NUM)
endif

cxxclean:
ifeq ($(LOSCFG_KERNEL_CPPSUPPORT), y)
	@-$(RM) $(LITEOSTOPDIR)/lib/cxxstl/c++
	@-$(RM) $(LITEOSTOPDIR)/lib/cxxstl/gccinclude
	@-$(RM) $(LITEOSTOPDIR)/lib/cxxstl/gdbinclude
	@-$(RM) $(LITEOSTOPDIR)/lib/a7_softfp_neon-vfpv4/*.a
	@-$(RM) $(LITEOSTOPDIR)/lib/a17_softfp_neon-vfpv4/*.a
	@-$(RM) $(LITEOSTOPDIR)/lib/armv5te_arm9_soft/*.a
	@-echo "clean cxx header finish"
endif

cclean:
	@-$(RM) $(LITEOSTOPDIR)/lib/a7_softfp_neon-vfpv4/*.a
	@-$(RM) $(LITEOSTOPDIR)/lib/a17_softfp_neon-vfpv4/*.a
	@-$(RM) $(LITEOSTOPDIR)/lib/armv5te_arm9_soft/*.a
	@-echo "clean c library finish"

.PHONY: cxxclean cclean
