all: kernel gnuefi clean iso run

kernel:
	make -C ../kernel kernel

gnuefi:
	make -C gnu-efi bootloader

img:
	dd if=/dev/zero of=boot.img bs=34664432 count=1
	mformat -F -i boot.img -f 1440 ::
	mmd -i boot.img ::/EFI
	mmd -i boot.img ::/EFI/BOOT
	cp gnu-efi/x86_64/bootloader/main.efi BOOTX64.EFI
	mcopy -i boot.img BOOTX64.EFI ::/EFI/BOOT
	mcopy -i boot.img startup.nsh ::
	mcopy -i boot.img zap.psf1 ::
	mcopy -i boot.img kernel ::

iso: img
	rm -rf iso
	mkdir iso
	cp boot.img iso
	xorriso -as mkisofs -R -f -e boot.img -no-emul-boot -o cdimage.iso iso
	rm -rf iso

run:
	qemu-system-x86_64 -drive file=deps/OVMF_CODE.fd,if=pflash,readonly=true,format=raw,unit=0 \
	-drive file=deps/OVMF_VARS.fd,if=pflash,readonly=false,format=raw \
	-smp 2 -m 12G -net none -cdrom cdimage.iso

clean:
	rm -rf out/exe/*
	rm -rf out/obj/*
	rm -f *.efi *.EFI
	rm -f *.o

.PHONY: clean kernel run iso img gnuefi all
