.PHONY: test build run debug

QEMU ?= qemu-system-x86_64
ISO ?= image.iso
RAMSIZE ?= 256M
_QEMU_FLAGS := -smp 4 -m $(RAMSIZE) -machine q35 -cpu SandyBridge,+avx2 \
				-cdrom $(ISO) -no-reboot -no-shutdown -usb \
				-drive if=pflash,format=raw,unit=0,file="qemu_fw/OVMF_CODE.fd",readonly=on \
				-drive if=pflash,format=raw,unit=1,file="qemu_fw/OVMF_VARS.fd" $(QEMU_FLAGS) \
				-usb -usbdevice keyboard -usbdevice mouse

main: build run

build:
	@echo "Building the kernel..."
	@$(MAKE) --quiet -C kernel
	@echo "Building the preloader..."
	@$(MAKE) --quiet -C preloader

debug:
	$(QEMU) $(_QEMU_FLAGS) -S -s

run:
	$(QEMU) -enable-kvm -cpu host $(_QEMU_FLAGS)

test:
	$(QEMU) $(_QEMU_FLAGS) -d int
