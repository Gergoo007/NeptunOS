RAMSIZE ?= 256M

ifeq ($(OS),Windows_NT)
QEMU_ACCEL ?= whpx,kernel-irqchip=off
else
QEMU_ACCEL ?= kvm
endif

QEMU_FLAGS_X86_64 := -cdrom image.iso -no-reboot -no-shutdown -m $(RAMSIZE) -M q35 $(QEMU_FLAGS) \
		-smp 1 -drive id=disk,file=disk.img,if=none -device pci-bridge,id=bridge0,chassis_nr=1 \
		-device ich9-usb-uhci6,bus=bridge0,id=uhci -device ich9-usb-uhci6,bus=bridge0,id=uhci2 \
		-device qemu-xhci,bus=bridge0,id=xhci -device ahci,id=ahci \
		-device ide-hd,drive=disk,bus=ahci.0 -device usb-mouse,bus=uhci.0 \
		-device usb-tablet,bus=xhci.0 -boot d -cpu SandyBridge $(QEMUFLAGS)

QEMU_FLAGS_X86_64_UEFI := -drive if=pflash,format=raw,unit=0,file="emu/OVMF/OVMF_CODE.fd",readonly=on \
		-drive if=pflash,format=raw,unit=1,file="emu/OVMF/OVMF_VARS.fd",readonly=on \
		$(QEMU_FLAGS_X86_64)

run: prepare_img
	qemu-system-x86_64 $(QEMU_FLAGS_X86_64) -accel $(QEMU_ACCEL)

uefi: prepare_img
	qemu-system-x86_64 $(QEMU_FLAGS_X86_64_UEFI) -accel $(QEMU_ACCEL)

aarch64: prepare_img_aarch64
	qemu-system-aarch64 -M raspi4b -kernel kernel/out/kernel8.img

test: prepare_img
	qemu-system-x86_64 $(QEMU_FLAGS_X86_64) -d int

debug:
	qemu-system-x86_64 $(QEMU_FLAGS_X86_64) -S -s \
		> /dev/null & gdb kernel/out/kernel --eval-command="target remote :1234"

bochs: prepare_img
	/usr/bin/bochs -dbg -qf emu/.bochsrc -rc emu/bochscmd

bochs2: prepare_img
	/opt/bochsgdb/bin/bochs -qf emu/.bochsrc2 \
		> /dev/null & gdb kernel/out/kernel --eval-command="target remote :1234"

prepare_img:
	@$(MAKE) -C kernel
	@cp kernel/out/kernel boot/kernel
	@xorriso -as mkisofs -R -r -J -b limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
		-apm-block-size 2048 --efi-boot limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		boot -o image.iso

prepare_img_aarch64:
	@ARCH=aarch64 $(MAKE) -C kernel
	@aarch64-none-elf-objcopy kernel/out/kernel -O binary kernel/out/kernel8.img

clean:
	@$(MAKE) -C kernel clean
	@$(MAKE) -C kernel/modules clean
