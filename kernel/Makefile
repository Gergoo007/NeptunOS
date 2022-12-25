rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CSRC := $(call rwildcard, src, *.c)
SSRC := $(call rwildcard, src, *.s)

COBJ := $(patsubst src/%.c, out/obj/%.c.o, $(CSRC))
SOBJ := $(patsubst src/%.s, out/obj/%.s.o, $(SSRC))

CC := gcc
AS := nasm
LD := ld

CFLAGS += -ffreestanding -fshort-wchar -g -fstack-protector-all
NASMFLAGS += -f elf64
LDFLAGS += -T script.ld -static -Bsymbolic -nostdlib

all: clean objdir $(COBJ) $(SOBJ) kernel

clean:
	rm -rf out/*

objdir: clean
	rsync -a -f"+ */" -f"- *" src/ out/obj/

out/obj/lib/tables/interrupts.c.o: src/lib/tables/interrupts.c
	$(CC) -ffreestanding -g -fstack-protector-all -mno-red-zone -mgeneral-regs-only -c $^ -o $@

out/obj/%.c.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

out/obj/%.s.o: src/%.s
	$(AS) $(NASMFLAGS) $^ -o $@

kernel: $(COBJ) $(SOBJ)
	$(LD) $(LDFLAGS) $^ -o ../bootloader/kernel

.PHONY: kernel all objdir clean