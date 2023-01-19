#!/bin/bash 

cd /mnt/et_cetera/projects/NeptunOS
make debug
#qemu-system-x86_64 -S -gdb tcp::1234 -bios /usr/share/qemu/bios-TianoCoreEFI.bin -smp 1 -m 2G -net none -no-reboot -no-shutdown -hda /mnt/et_cetera/projects/NeptunOS/bootloader/boot.img
