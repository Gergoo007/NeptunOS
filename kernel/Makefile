rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CSRC := $(call rwildcard, src/neptunos, *.c)
SSRC := $(call rwildcard, src/neptunos, *.s)

COBJ := $(patsubst src/neptunos/%.c, out/obj/neptunos/%.c.o, $(CSRC))
SOBJ := $(patsubst src/neptunos/%.s, out/obj/neptunos/%.s.o, $(SSRC))

CC = gcc
AS = as
LD = ld.gold

_CFLAGS += -ffreestanding -fshort-wchar -Isrc -nostdinc \
	-Wno-interrupt-service-routine -Wno-builtin-requires-header \
	-Wno-incompatible-library-redeclaration -Wno-long-long -std=c17 \
	$(CFLAGS)
_LDFLAGS += -T link.ld -Bsymbolic -nostdlib -z max-page-size=0x1000 -static $(LDFLAGS)

k_all: k_clean info objdir $(COBJ) $(SOBJ) kernel

info:
	@echo AS: $(AS), CC: $(CC), LD: $(LD)

k_clean:
	rm -rf out/*

objdir:
	rsync -a -f"+ */" -f"- *" src/neptunos out/obj/

out/obj/font.o:
	$(LD) -r -b binary -o $@ font.psf

out/obj/neptunos/tables/interrupts.c.o: src/neptunos/tables/interrupts.c
	$(CC) $(_CFLAGS) -mgeneral-regs-only -c $^ -o $@

out/obj/neptunos/graphics/panic.c.o: src/neptunos/graphics/panic.c
	$(CC) $(_CFLAGS) -mgeneral-regs-only -c $^ -o $@

out/obj/neptunos/serial/com.c.o: src/neptunos/serial/com.c
	$(CC) $(_CFLAGS) -mgeneral-regs-only -c $^ -o $@

out/obj/neptunos/hw_interrupts/pic.c.o: src/neptunos/hw_interrupts/pic.c
	$(CC) $(_CFLAGS) -mgeneral-regs-only -c $^ -o $@

out/obj/%.c.o: src/%.c
	$(CC) $(_CFLAGS) -c $^ -o $@

out/obj/%.s.o: src/%.s
	$(AS) $(NASMFLAGS) $^ -o $@

kernel: $(COBJ) $(SOBJ) out/obj/font.o
	$(LD) $(_LDFLAGS) $^ -o out/kernel
	ld -r -b binary -o ../multiboot_pre/out/payload.o out/kernel

.PHONY: kernel k_all objdir k_clean no_debug
