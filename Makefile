run: prepare_img
	qemu-system-x86_64 -cdrom myos.img -no-reboot -no-shutdown \
		-drive if=pflash,format=raw,unit=0,file="OVMF/OVMF_CODE.fd",readonly=on \
		-drive if=pflash,format=raw,unit=1,file="OVMF/OVMF_VARS.fd" -m 4G -smp 4

prepare_img: build
	@bootstuff/mkbootimg bootstuff/config.json myos.img

build:
	@$(MAKE) --no-print-directory -C kernel
	@cp kernel/out/kernel bootstuff/initrd/kernel
