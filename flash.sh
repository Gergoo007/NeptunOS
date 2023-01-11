#!/bin/bash

mount /dev/sdd1 /mnt/misc
cp bootloader/BOOTX64.EFI /mnt/misc/EFI/BOOT/BOOTX64.EFI
cp kernel/out/kernel bootloader/startup.nsh /mnt/misc/

