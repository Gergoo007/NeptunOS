#!/bin/bash 

cd /mnt/et_cetera/projects/SymbiOSys
make -C bootloader kernel gnuefi clean iso
qemu-system-x86_64 -S -gdb tcp::1234 -drive file=bootloader/deps/OVMF_CODE.fd,if=pflash,readonly=true,format=raw,unit=0 -drive file=bootloader/deps/OVMF_VARS.fd,if=pflash,readonly=false,format=raw -m 8G -net none -cdrom bootloader/cdimage.iso
