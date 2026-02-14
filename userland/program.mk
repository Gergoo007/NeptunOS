include ../common.mk

install: link
	echo "  > [App] $(PROGRAMNAME)"
	../../tools/mount.sh
	cp $(PROGRAMNAME) /mnt/misc/bin/
	sync
