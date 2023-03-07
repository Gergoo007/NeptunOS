.PHONY: all clean link build pack uefi run

CFLAGS := -c -nostdinc -nostdlib -ffreestanding -Isrc -std=c17 -xc
LDFLAGS := -n -T src/link.ld

RAMSIZE ?= 2G
QEMU_ARGS ?= -smp 1 -m $(RAMSIZE) \
	-net none -no-reboot -no-shutdown \
	-cdrom out.iso

ASM_SRC := $(shell find . -name '*.s' -type f)
ASM_OBJ = $(patsubst ./src/%.s, out/%.s.o, $(ASM_SRC))

C_SRC := $(shell find . -name '*.c' -type f)
C_OBJ = $(patsubst ./src/%.c, out/%.c.o, $(C_SRC))

all: build pack uefi

clean:
	@rm -rf out/*
	@rsync -a --include='*/' --exclude='*' src/ out/

out/%.c.o: src/%.c
	gcc $(CFLAGS) $^ -o $@

out/%.s.o: src/%.s
	nasm -felf64 $^ -o $@
#	as -x assembler-with-cpp --32 $^ -o $@

link: clean $(C_OBJ) $(ASM_OBJ)
	ld $(LDFLAGS) $(ASM_OBJ) $(C_OBJ) -o out/kernel.elf

build: link
	./verify.sh out/kernel.elf
	cp out/kernel.elf iso_content/

pack:
	grub-mkrescue iso_content -o out.iso

uefi:
	qemu-system-x86_64 -bios ovmf/OVMF_CODE.fd $(QEMU_ARGS)

run:
	qemu-system-x86_64 $(QEMU_ARGS)
