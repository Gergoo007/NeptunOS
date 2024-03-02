.PHONY: all clean kernel store

CSRC :=	$(shell find src -name '*.c' -printf '%P\n')
SSRC :=	$(shell find src -name '*.s' -printf '%P\n')

COBJS := $(patsubst %.c,out/%.c.o,$(CSRC))
SOBJS := $(patsubst %.s,out/%.s.o,$(SSRC))

CC ?= cc
AS ?= as
LD ?= ld.gold

CFLAGS := -xc -std=gnu17 -ffreestanding -nostdinc -nostdlib -mno-red-zone -mcmodel=large -Wshadow \
	-Wall -Wextra -Isrc -fno-stack-protector -mgeneral-regs-only -Wno-unused-parameter -fno-omit-frame-pointer \
	-g -O0 #-Wno-unused-function
LDFLAGS := -g -static -T ldscript.ld -z max-page-size=0x200000

all: clean .WAIT kernel store

clean:
	@rm -rf out/*
	@find src -type d -exec mkdir -p "out/{}" \;
	@mv out/src/* out/
	@rmdir out/src

out/%.c.o: src/%.c
	@$(CC) $(CFLAGS) -c $^ -o $@
	@echo -e "  > $(CC) $^"

out/%.s.o: src/%.s
	@$(AS) -c $^ -o $@
	@echo -e "  > $(AS) $^"

out/font.o: zap-ext-light24.psf
	@$(LD) -r -b binary -o $@ $^
	@echo -e "  > $(LD) $^"

kernel: $(COBJS) $(SOBJS) out/font.o
	@$(LD) $(LDFLAGS) $^ -o out/kernel
	@echo -e "  > $(LD) out/kernel"

store: kernel
	@$(LD) -r -b binary -o out/payload.o out/kernel
	@echo -e "  > $(LD) out/payload.o"
