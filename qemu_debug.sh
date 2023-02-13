#!/bin/bash 

cd /mnt/et_cetera/projects/SymbiOSys
make build
qemu-system-x86_64 -S -gdb tcp::1234 -drive file=bootloader/ovmf/OVMF_CODE.fd,if=pflash,readonly=true,format=raw,unit=0 -drive file=bootloader/ovmf/OVMF_VARS.fd,if=pflash,readonly=false,format=raw -m 256M -net none -cdrom bootloader/disk.iso
