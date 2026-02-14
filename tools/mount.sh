DIR="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if ! mountpoint -q /mnt/misc; then
	dev=$(doas losetup --find --show --partscan ${DIR}/../disk.img)
	mntpoint=${1:-/mnt/misc}
	doas mount -o gid=$(id -g),umask=0002 ${dev}p1 $mntpoint
fi
