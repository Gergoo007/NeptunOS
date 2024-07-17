if [[ -b "$1" ]]; then
	make build
	doas mount $1 /mnt
    doas cp kernel/out/kernel /mnt/kernel
    doas umount $1
	exit 0
else
	echo "Az eszköz nem létezik!"
	exit 1
fi

