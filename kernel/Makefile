rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CSRC := $(call rwildcard, src/neptunos, *.c)
SSRC := $(call rwildcard, src/neptunos, *.s)

COBJ := $(patsubst src/neptunos/%.c, out/obj/neptunos/%.c.o, $(CSRC))
SOBJ := $(patsubst src/neptunos/%.s, out/obj/neptunos/%.s.o, $(SSRC))

CC = clang
AS = nasm
LD = ld.gold

CFLAGS += -ffreestanding -fshort-wchar -g -fno-stack-protector -I/mnt/et_cetera/projects/NeptunOS/kernel/src -nostdinc -Wno-interrupt-service-routine
NASMFLAGS += -f elf64
LDFLAGS += -T script.ld -static -Bsymbolic -nostdlib -z max-page-size=0x1000

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
	$(CC) $(CFLAGS) -mgeneral-regs-only -I/mnt/et_cetera/projects/NeptunOS/kernel/src -c $^ -o $@

out/obj/%.c.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

out/obj/%.s.o: src/%.s
	$(AS) $(NASMFLAGS) $^ -o $@

kernel: $(COBJ) $(SOBJ) out/obj/font.o
	$(LD) $(LDFLAGS) $^ -o out/kernel

no_debug: k_all
	strip out/kernel

.PHONY: kernel k_all objdir k_clean no_debug
