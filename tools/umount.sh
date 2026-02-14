if mountpoint -q /mnt/misc; then
	doas losetup -D
	doas umount /mnt/misc
fi
