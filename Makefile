run: prepare_img
	qemu-system-x86_64 -cdrom arzene.iso -no-reboot -no-shutdown \
		-drive if=pflash,format=raw,unit=0,file="OVMF/OVMF_CODE.fd",readonly=on \
		-drive if=pflash,format=raw,unit=1,file="OVMF/OVMF_VARS.fd" -m 4G -smp 4

test:
	qemu-system-x86_64 -cdrom arzene.iso -no-reboot -no-shutdown \
		-drive if=pflash,format=raw,unit=0,file="OVMF/OVMF_CODE.fd",readonly=on \
		-drive if=pflash,format=raw,unit=1,file="OVMF/OVMF_VARS.fd" -m 4G -smp 4 -d int

prepare_img: build
	@bootstuff/mkbootimg bootstuff/config.json arzene.iso

build:
	@$(MAKE) --no-print-directory -C kernel
	@cp kernel/out/kernel bootstuff/initrd/kernel
