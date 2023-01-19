neptunos:
	make -C kernel k_all
	make -C bootloader bl_all

build:
	make -C kernel k_all
	make -C bootloader bl_clean all img

debug:
#	make -C kernel k_all
	make -C bootloader bl_debug
