.PHONY: clean

CSRC :=	$(shell find src -name '*.c' -printf '%P\n')
SSRC :=	$(shell find src -name '*.s' -printf '%P\n')

COBJS := $(patsubst %.c,out/%.c.o,$(CSRC))
SOBJS := $(patsubst %.s,out/%.s.o,$(SSRC))

CC ?= cc
AS ?= as
LD ?= ld.gold

CFLAGS := -xc -std=gnu17 -ffreestanding -nostdinc -nostdlib -mno-red-zone \
	-Wall -Wextra -Isrc -Wno-unused-parameter -mgeneral-regs-only -fPIE
LDFLAGS := -static -T src/ldscript.ld

package: preloader
	@cp out/preloader disk/kernel
	@grub-mkrescue disk -o ../image.iso --quiet

preloader: $(COBJS) $(SOBJS)
	@$(LD) $(LDFLAGS) $(COBJS) $(SOBJS) ../kernel/out/payload.o -o out/preloader
	@echo -e "  > $(LD) out/preloader"

out/%.c.o: src/%.c clean
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo -e "  > $(CC) $<"

out/%.s.o: src/%.s clean
	@$(AS) -c $< -o $@
	@echo -e "  > $(AS) $<"

clean:
	@find out -name "*.o" -type f -delete
