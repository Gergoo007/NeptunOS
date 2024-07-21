if [[ -b "$1" ]]; then
	make build
	doas mount $1 /mnt

	# cpio archívum initrd-nek
	echo "kernel/out/kernel" | cpio -o > bootstuff/initrd.cpio
	doas cp bootstuff/initrd.cpio /mnt/BOOTBOOT/INITRD

	doas umount $1
	sync
	exit 0
else
	echo "Az eszköz nem létezik!"
	exit 1
fi
