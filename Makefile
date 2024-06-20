.PHONY: test build run debug

QEMU ?= qemu-system-x86_64
ISO ?= image.iso
RAMSIZE ?= 256M
# Virtuális gép EHCI, UHCI és OHCI kontrollerrel
# EHCI-re kötve: tablet
# OHCI-re kötve: egér
# UHCI-re kötve: billentyűzet
_QEMU_FLAGS := -smp 4 -m $(RAMSIZE) -machine q35 -cpu SandyBridge,+avx2 \
				-cdrom $(ISO) -no-reboot -no-shutdown \
				-drive if=pflash,format=raw,unit=0,file="qemu_fw/OVMF_CODE.fd",readonly=on \
				-drive if=pflash,format=raw,unit=1,file="qemu_fw/OVMF_VARS.fd" $(QEMU_FLAGS) \
				-device usb-ehci,id=ehci -device ich9-usb-uhci1,id=uhci -device pci-ohci,id=ohci \
				-device usb-tablet,bus=ehci.0 -device usb-mouse,bus=ohci.0 \
				-drive id=disk,file=disk.img,if=none \
				-device ahci,id=ahci \
				-device ide-hd,drive=disk,bus=ahci.0

main: returner build run

returner:
	make -C userspace/returner

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
