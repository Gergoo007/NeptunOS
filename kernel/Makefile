rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CSRC := $(call rwildcard, src/neptunos, *.c)
SSRC := $(call rwildcard, src/neptunos, *.s)

COBJ := $(patsubst src/neptunos/%.c, out/obj/neptunos/%.c.o, $(CSRC))
SOBJ := $(patsubst src/neptunos/%.s, out/obj/neptunos/%.s.o, $(SSRC))

CC := gcc
AS := nasm
LD := ld

CFLAGS += -ffreestanding -fshort-wchar -g -fno-stack-protector -I/mnt/et_cetera/projects/NeptunOS/kernel/src -nostdinc
NASMFLAGS += -f elf64
LDFLAGS += -T script.ld -static -Bsymbolic -nostdlib -n -z max-page-size=0x1000

kernel_all: clean objdir $(COBJ) $(SOBJ) kernel

clean:
	rm -rf out/*

objdir:
	rsync -a -f"+ */" -f"- *" src/neptunos out/obj/

out/obj/font.o:
	$(LD) -r -b binary -o $@ font.psf

out/obj/neptunos/tables/interrupts.c.o: src/neptunos/tables/interrupts.c
	$(CC) -ffreestanding -g -fno-stack-protector -mno-red-zone -mgeneral-regs-only -I/mnt/et_cetera/projects/NeptunOS/kernel/src -c $^ -o $@

out/obj/%.c.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

out/obj/%.s.o: src/%.s
	$(AS) $(NASMFLAGS) $^ -o $@

kernel: $(COBJ) $(SOBJ) out/obj/font.o
	$(LD) $(LDFLAGS) $^ -o out/kernel

no_debug: kernel_all
	strip out/kernel

.PHONY: kernel all objdir clean no_debug
