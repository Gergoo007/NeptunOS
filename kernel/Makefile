# Disable idiotic invisible default variables like CC
MAKEFLAGS += --no-builtin-variables

ARCH ?= amd64

CCSRCS += $(subst src/,,$(shell find src -name *.cc -not -path 'src/arch/*' -not -path '*bakexclude*')) arch/arch.cc

MANAGE_ARCH_STUFF := 1
include $(ARCH).mk

CCOBJS += $(patsubst %.cc,out/%.cc.o,$(CCSRCS))
CCDEPS += $(patsubst %.cc,out/%.cc.d,$(CCSRCS))

# Ha ezek a fájlok változnak, az egész kernel újra lesz fordítva
EXTRADEPS := Makefile amd64.mk aarch64.mk

kernel:
# $(ARCH) fájl létrehozása, ez mutatja,
# hogy az előző build objektum fájljai
# kompatibilisek-e a mostaniéval
ifeq ($(ARCH),amd64)
	-test -f out/aarch64 && $(MAKE) clean
else ifeq ($(ARCH),aarch64)
	-test -f out/amd64 && $(MAKE) clean
endif
	touch out/$(ARCH)
	$(MAKE) --no-print-directory modules
	$(MAKE) --no-print-directory link

link: $(SOBJS) $(CCOBJS) out/font.o
	echo "  > $(LD) $@"
	$(LD) $(LDFLAGS) -T kernel.$(ARCH).ld -static $^ -o out/kernel

modules:
	$(MAKE) -C modules

-include $(CCDEPS)

out/%.cc.o: src/%.cc $(EXTRADEPS)
	echo "  > $(CC) $<"
	$(CC) $(CXXFLAGS) -c $< -o $@

out/%.s.o: src/%.s $(EXTRADEPS)
	echo "  > $(AS) $<"
	$(AS) $(ASFLAGS) -c $< -o $@

out/font.o: src/font.psf $(EXTRADEPS)
	echo "  > $(LD) $<"
	$(LD) -r -b binary $< -o $@

clean:
	rm -rf out/*
	find src -type d -exec mkdir -p "out/{}" \;
	cp -r out/src/* out/
	rm -rf out/src

.PHONY: modules
$(V).SILENT:
