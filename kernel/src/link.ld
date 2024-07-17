ENTRY(kmain)

mmio		= 0xffffffffc0000000; # -1024M [32M]
fb			= 0xffffffffc2000000; # -992M [128M, eleg egy 8K-s kepnek]
bootboot	= 0xffffffffca000000; # -864M [32M]
environment = 0xffffffffcc000000; # -832M [32M]

SECTIONS {
	. = 0xffffffffce000000; # -800M

	.text : {
		*(.text)
	}

	.data : {
		*(.data)
		*(.rodata)
	}

	.bss : {
		*(.bss)
	}
}
