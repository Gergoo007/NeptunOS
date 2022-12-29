all: build qemu

build:
	make -C kernel kernel_all
	make -C bootloader all

qemu:
	make -C bootloader run
