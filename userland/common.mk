# Disable idiotic invisible default variables like CC
MAKEFLAGS += --no-builtin-variables

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CCSRCS := $(subst src/,,$(call rwildcard,src,*.cc))
CCOBJS := $(patsubst %.cc,out/%.cc.o,$(CCSRCS))
CCDEPENDS := $(patsubst %.cc,out/%.cc.d,$(CCSRCS))

SSRCS := $(subst src/,,$(call rwildcard,src,*.s))
SOBJS := $(patsubst %.s,out/%.s.o,$(SSRCS))

CC := g++
LD := ld
AS := gcc

CXXFLAGS := -std=gnu++23 -Wshadow -O0 -g -Wno-multichar -Isrc -MMD -MP -mavx2 -ffreestanding -nostdlib -nostdinc -I..
ASFLAGS := -ffreestanding -nostdlib -nostdinc
LDFLAGS := -g -T ../userprogram.ld

PROGRAMNAME := $(notdir $(CURDIR))
OUTPUTNAME := $(PROGRAMNAME)

link: deps $(CCOBJS) $(SOBJS)
	$(LD) $(CCOBJS) $(SOBJS) $(foreach x,$(DEPENDENCIES),../$(x)/$(x).o) $(LDFLAGS) -o $(OUTPUTNAME)

deps:
	@for d in $(DEPENDENCIES); do \
		echo "  > [Dep] $$d"; \
		$(MAKE) --no-print-directory -C ../$$d link || exit 1; \
	done

-include $(CCDEPENDS) $(CPPDEPENDS)

out/%.cc.o: src/%.cc Makefile
	echo "  > $(CC) $<"
	$(CC) -c $< $(CXXFLAGS) -o $@

out/%.s.o: src/%.s Makefile
	echo "  > $(AS) $<"
	$(AS) -c $< $(ASFLAGS) -o $@

clean:
	rm -rf out/*
	find src -type d -exec mkdir -p "out/{}" \;
	cp -r out/src/* out/
	rm -rf out/src

.PHONY: modules
$(V).SILENT:
