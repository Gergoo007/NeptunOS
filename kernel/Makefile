# Disable idiotic invisible default variables like CC
MAKEFLAGS += --no-builtin-variables

ARCH ?= AMD64

CCSRCS += $(subst src/,,$(shell find src -name *.cc -not -path 'src/arch/*')) arch/arch.cc

ifeq ($(ARCH),AMD64)
include amd64.mk
else ifeq ($(ARCH),AARCH64)
include aarch64.mk
else
$(error Available targets: AMD64, AARCH64)
endif

CCOBJS += $(patsubst %.cc,out/%.cc.o,$(CCSRCS))
CCDEPS += $(patsubst %.cc,out/%.cc.d,$(CCSRCS))

# Ha ezek a fájlok változnak, az egész kernel újra lesz fordítva
EXTRADEPS := Makefile amd64.mk aarch64.mk

CXXFLAGS += -MMD -MP -DDEBUG

kernel:
# $(ARCH) fájl létrehozása, ez mutatja,
# hogy az előző build objektum fájljai
# kompatibilisek-e a mostaniéval
ifeq ($(ARCH),AMD64)
	-test -f out/AARCH64 && $(MAKE) clean
else ifeq ($(ARCH),AARCH64)
	-test -f out/AMD64 && $(MAKE) clean
endif
	touch out/$(ARCH)
	$(MAKE) --no-print-directory link
	$(MAKE) --no-print-directory modules

link: $(SOBJS) $(CCOBJS) out/font.o
	echo "  > $(LD) $@"
	echo $(SOBJS)
	$(LD) $(LDFLAGS) $^ -o out/kernel

modules:
#	$(MAKE) -C modules

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
