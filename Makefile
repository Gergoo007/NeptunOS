.PHONY: all clean link build pack uefi run

CFLAGS := -c -nostdinc -nostdlib -ffreestanding -Isrc -std=c17 -O0
LDFLAGS := -n -T src/link.ld

RAMSIZE ?= 2G
QEMU_ARGS ?= -smp 1 -m $(RAMSIZE) \
	-net none -no-reboot -no-shutdown \
	-cdrom out.iso

ASM_SRC := $(shell find . -not -path "./multiboot_pre/*" -name '*.S' -type f)
ASM_OBJ = $(patsubst ./src/%.S, out/%.s.o, $(ASM_SRC))

C_SRC := $(shell find . -not -path "./multiboot_pre/*" -name '*.c' -type f)
C_OBJ = $(patsubst ./src/%.c, out/%.c.o, $(C_SRC))

all: build pack uefi

clean:
	@rm -rf out/*
	@rsync -a --include='*/' --exclude='*' src/ out/

out/%.c.o: src/%.c
	gcc $(CFLAGS) -xc $^ -o $@

out/%.s.o: src/%.S
	as $^ -o $@

link: clean $(C_OBJ) $(ASM_OBJ)
	ld $(LDFLAGS) $(ASM_OBJ) $(C_OBJ) -o out/post

build: link
	ld -r -b binary -o multiboot_pre/out/payload.o out/post

pack:
	make -C multiboot_pre
	grub-mkrescue iso_content -o out.iso

uefi:
	qemu-system-x86_64 -bios ovmf/OVMF_CODE.fd $(QEMU_ARGS)

run:
	qemu-system-x86_64 $(QEMU_ARGS)

debug_uefi:
	qemu-system-x86_64 -bios ovmf/OVMF_CODE.fd $(QEMU_ARGS) -s -S

debug:
	qemu-system-x86_64 $(QEMU_ARGS) -s -S
