.PHONY: test build run

QEMU ?= qemu-system-x86_64
ISO ?= image.iso
_QEMU_FLAGS_UEFI := -enable-kvm -machine q35 -cpu SandyBridge,+avx2 \
				-cdrom $(ISO) \
				-drive if=pflash,format=raw,unit=0,file="qemu_fw/OVMF_CODE.fd",readonly=on \
				-drive if=pflash,format=raw,unit=1,file="qemu_fw/OVMF_VARS.fd" $(QEMU_FLAGS)

_QEMU_FLAGS_BIOS := -enable-kvm -machine q35 -cpu SandyBridge,+avx2 -cdrom $(ISO)

test: run

build:
	$(MAKE) -C kernel
	$(MAKE) -C preloader

run: build
	$(QEMU) $(_QEMU_FLAGS_UEFI)

bios: build
	$(QEMU) $(_QEMU_FLAGS_BIOS)
