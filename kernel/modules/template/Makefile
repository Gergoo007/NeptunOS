MANAGE_ARCH_STUFF := 0
ARCH ?= amd64
include ../../$(ARCH).mk

CXXFLAGS += -I../../src -DIS_MODULE

CCSRCS += $(subst src/,,$(shell find src -name '*.cc' -not -path '*bakexclude*'))

CCOBJS += $(patsubst %.cc,out/%.cc.o,$(CCSRCS))
CCDEPS += $(patsubst %.cc,out/%.cc.d,$(CCSRCS))
EXTRADEPS := Makefile ../template/Makefile

MOD_NAME := $(notdir $(CURDIR))

module:
ifeq ($(ARCH),amd64)
	-test -f out/aarch64 && $(MAKE) clean
else ifeq ($(ARCH),aarch64)
	-test -f out/amd64 && $(MAKE) clean
endif
	touch out/$(ARCH)
	$(MAKE) --no-print-directory link

link: $(SOBJS) $(CCOBJS) $(EXTRADEPS)
# 	echo "  > $(LD) $@"
	$(LD) $(LDFLAGS) -T ../module.$(ARCH).ld -r $(SOBJS) $(CCOBJS) -o out/$(MOD_NAME).ko
	$(STRIP) -d out/$(MOD_NAME).ko

-include $(CCDEPS)

out/%.cc.o: src/%.cc $(EXTRADEPS)
# 	echo "  > $(CC) $<"
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf out/*
	find src -type d -exec mkdir -p "out/{}" \;
	-cp -r out/src/* out/
	rm -rf out/src

$(V).SILENT:
