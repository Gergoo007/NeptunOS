# define which architecture you're targeting
ARCH = x86_64
# define your target file here
TARGET = BOOTX64.EFI
#define your sources here
SRCS = $(wildcard *.c)
# define your default compiler flags
CFLAGS = -pedantic -Wall -Wextra -Werror --ansi -O2
# define your default linker flags
#LDFLAGS =
# define your additional libraries here
#LIBS = -lm

_all: kernel all run

include uefi/Makefile

kernel:
	make -C ../kernel

boot.img:
	dd if=/dev/zero of=boot.img bs=34664432 count=1

img: boot.img
	mformat -F -i boot.img -f 1440 ::
	mmd -i boot.img ::/EFI
	mmd -i boot.img ::/EFI/BOOT
	mcopy -i boot.img BOOTX64.EFI ::/EFI/BOOT
	mcopy -i boot.img startup.nsh ::
	mcopy -i boot.img ../kernel/out/kernel ::

iso: img
	rm -rf iso
	mkdir iso
	cp boot.img iso
	xorriso -as mkisofs -R -f -e boot.img -no-emul-boot -o cdimage.iso iso
	rm -rf iso

run: img
	qemu-system-x86_64 -bios /usr/share/qemu/bios-TianoCoreEFI.bin \
	-serial stdio -smp 1 -m 256M -net none -no-reboot -no-shutdown \
	-hda boot.img

run_iso: iso
	qemu-system-x86_64 -bios /usr/share/qemu/bios-TianoCoreEFI.bin \
	-serial stdio -smp 1 -m 256M -net none -no-reboot -no-shutdown \
	-hda boot.img

.PHONY: _all run iso img
