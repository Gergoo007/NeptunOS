.PHONY: all clean preloader package

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

all: clean preloader package

clean:
	@find out -name "*.o" -type f -delete

out/%.c.o: src/%.c
	@$(CC) $(CFLAGS) -c $^ -o $@
	@echo -e "  > $(CC) $^"

out/%.s.o: src/%.s
	@$(AS) -c $^ -o $@
	@echo -e "  > $(AS) $^"

preloader: $(COBJS) $(SOBJS)
	@$(LD) $(LDFLAGS) $^ ../kernel/out/payload.o -o out/preloader
	@echo -e "  > $(LD) $^ out/preloader"

package:
	@cp out/preloader disk/kernel
	@grub-mkrescue disk -o ../image.iso --quiet
