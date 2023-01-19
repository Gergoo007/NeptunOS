ARCH = x86_64

TARGET = BOOTX64.EFI

SRCS = $(wildcard *.c)

RAMSIZE ?= 2G

CFLAGS = -O0

bl_all: bl_clean all img run

include uefi/Makefile

flags:
	@echo
	@echo CFLAGS: $(CFLAGS)
	@echo
	@echo LDFLAGS: $(LDFLAGS)
	@echo

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
	qemu-system-x86_64 -bios /usr/share/qemu/bios-TianoCoreEFI.bin -machine q35 \
	-serial stdio -smp 1 -m $(RAMSIZE) -net none -no-reboot -no-shutdown -enable-kvm \
	-hda boot.img

run_iso: iso
	qemu-system-x86_64 -bios /usr/share/qemu/bios-TianoCoreEFI.bin \
	-serial stdio -smp 1 -m $(RAMSIZE) -net none -no-reboot -no-shutdown \
	-hda boot.img

bl_debug: bl_clean all img
	qemu-system-x86_64 -S -gdb tcp::1234 -bios /usr/share/qemu/bios-TianoCoreEFI.bin \
	-serial stdio -smp 1 -m $(RAMSIZE) -net none -no-reboot -no-shutdown \
	-hda boot.img

bl_clean:
	rm -rf *.o *.iso *.img

.PHONY: bl_all bl_clean run iso img bl_debug
