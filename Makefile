.PHONY: test build run

QEMU ?= qemu-system-x86_64
ISO ?= image.iso
RAMSIZE ?= 256M
_QEMU_FLAGS_UEFI := -m $(RAMSIZE) -enable-kvm -machine q35 -cpu SandyBridge,+avx2 \
				-cdrom $(ISO) -no-reboot -no-shutdown \
				-drive if=pflash,format=raw,unit=0,file="qemu_fw/OVMF_CODE.fd",readonly=on \
				-drive if=pflash,format=raw,unit=1,file="qemu_fw/OVMF_VARS.fd" $(QEMU_FLAGS)

_QEMU_FLAGS_DEBUG := -m $(RAMSIZE) -d int -machine q35 -cpu SandyBridge,+avx2 \
				-cdrom $(ISO) -no-reboot -no-shutdown \
				-drive if=pflash,format=raw,unit=0,file="qemu_fw/OVMF_CODE.fd",readonly=on \
				-drive if=pflash,format=raw,unit=1,file="qemu_fw/OVMF_VARS.fd" $(QEMU_FLAGS)

test: debug

build:
	$(MAKE) -C kernel
	$(MAKE) -C preloader

debug: build
	$(QEMU) $(_QEMU_FLAGS_DEBUG)

run: build
	$(QEMU) $(_QEMU_FLAGS_UEFI)
