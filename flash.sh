if [[ -b "$1" ]]; then
	make build
	doas dd if=image.iso of=$1 bs=4M
	exit 0
else
	echo "Az eszköz nem létezik!"
	exit 1
fi

