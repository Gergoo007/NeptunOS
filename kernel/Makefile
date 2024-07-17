rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CSRCS := $(subst src/,,$(call rwildcard,src,*.c))
COBJS := $(patsubst %.c,out/%.c.o,$(CSRCS))

SSRCS := $(subst src/,,$(call rwildcard,src,*.s))
SOBJS := $(patsubst %.s,out/%.s.o,$(SSRCS))

CC ?= gcc
AS ?= as
LD ?= ld.gold

CFLAGS := -mgeneral-regs-only -ffreestanding -nostdlib -nostdinc \
	-Isrc -xc -std=gnu17 -Wall -Wextra -Wshadow -Wno-address-of-packed-member \
	-Wno-incompatible-library-redeclaration -Wno-unused-parameter -fno-stack-protector
ASFLAGS :=
LDFLAGS := -T src/link.ld

kernel: clean
	@$(MAKE) --no-print-directory link

link: $(SOBJS) $(COBJS) out/font.o
	@echo "  > $(LD) $@"
	@$(LD) $(LDFLAGS) out/*.o -o out/kernel

out/%.c.o: src/%.c
	@echo "  > $(CC) $^"
	@$(CC) $(CFLAGS) -c $^ -o out/$(notdir $@)

out/%.s.o: src/%.s
	@echo "  > $(AS) $^"
	@$(AS) $(ASFLAGS) -c $^ -o out/$(notdir $@)

out/font.o: src/font.psf
	@echo "  > $(LD) $^"
	@$(LD) -r -b binary $^ -o $@

clean:
	@rm -rf out
	@mkdir out
