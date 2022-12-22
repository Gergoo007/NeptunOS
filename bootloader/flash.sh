#!/bin/bash

mount /dev/sdd1 /mnt/misc
cp gnu-efi/x86_64/bootloader/main.efi /mnt/misc/EFI/BOOT/BOOTX64.EFI
cp kernel zap.psf1 startup.nsh /mnt/misc/
