all: build qemu

build:
	make -C kernel k_all
	make -C bootloader bootloader_all

qemu:
	make -C bootloader qemu
