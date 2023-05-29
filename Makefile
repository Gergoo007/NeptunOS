.PHONY: all clean link build pack uefi run

RAMSIZE ?= 2G
QEMU_ARGS ?= -smp 1 -machine q35 -m $(RAMSIZE) \
	-net none -no-reboot -no-shutdown \
	-cdrom out.iso -drive file=raw.img,format=raw

all: _kernel preloader pack uefi

_kernel:
	make -C kernel

preloader:
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
