MKBOOTIMG := bootboot/mkbootimg/mkbootimg

RAMSIZE ?= 2G

_QEMU_ARGS := -bios ovmf/OVMF_CODE.fd -machine q35 -serial stdio -smp 1 -m $(RAMSIZE) \
	-net none -no-reboot -no-shutdown $(QEMU_ARGS)

bootloader_all: get_kernel boot_image

get_kernel:
	cp ../kernel/out/kernel initrd/kernel
	$(MKBOOTIMG) check initrd/kernel

boot_image:
	$(MKBOOTIMG) config.json disk.iso

qemu:
	qemu-system-x86_64 $(_QEMU_ARGS) -cdrom disk.iso
