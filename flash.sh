if [[ -b "$1" ]]; then
	make build
	doas dd if=myos.img of=$1 bs=4M conv=fsync
	exit 0
else
	echo "Az eszköz nem létezik!"
	exit 1
fi

