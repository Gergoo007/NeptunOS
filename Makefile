.PHONY: test build run debug

QEMU ?= qemu-system-x86_64
ISO ?= image.iso
RAMSIZE ?= 256M
_QEMU_FLAGS_UEFI_TEST := -smp 4 -m $(RAMSIZE) -machine q35 -cpu SandyBridge,+avx2 \
				-cdrom $(ISO) -no-reboot -no-shutdown -d int \
				-drive if=pflash,format=raw,unit=0,file="qemu_fw/OVMF_CODE.fd",readonly=on \
				-drive if=pflash,format=raw,unit=1,file="qemu_fw/OVMF_VARS.fd" $(QEMU_FLAGS)

_QEMU_FLAGS_UEFI := -smp 4 -m $(RAMSIZE) -enable-kvm -machine q35 -cpu SandyBridge,+avx2 \
				-cdrom $(ISO) -no-reboot -no-shutdown \
				-drive if=pflash,format=raw,unit=0,file="qemu_fw/OVMF_CODE.fd",readonly=on \
				-drive if=pflash,format=raw,unit=1,file="qemu_fw/OVMF_VARS.fd" $(QEMU_FLAGS)

_QEMU_FLAGS_DEBUG := -smp 4 -m $(RAMSIZE) -d int -machine q35 -cpu SandyBridge,+avx2 \
				-cdrom $(ISO) -no-reboot -no-shutdown \
				-drive if=pflash,format=raw,unit=0,file="qemu_fw/OVMF_CODE.fd",readonly=on \
				-drive if=pflash,format=raw,unit=1,file="qemu_fw/OVMF_VARS.fd" $(QEMU_FLAGS)

main: run

build:
	@echo "Building the kernel..."
	@$(MAKE) --quiet -C kernel
	@echo "Building the preloader..."
	@$(MAKE) --quiet -C preloader

debug: build
	$(QEMU) $(_QEMU_FLAGS_DEBUG)

run: build
	$(QEMU) $(_QEMU_FLAGS_UEFI)

test: build
	$(QEMU) $(_QEMU_FLAGS_UEFI_TEST)
