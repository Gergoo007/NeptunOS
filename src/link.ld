OUTPUT_FORMAT(elf64-x86-64)
ENTRY(onboot)

SECTIONS {
	. = 1M;
	.multiboot : ALIGN(8) {
		*(.multiboot)
	}
	.text : ALIGN(CONSTANT(MAXPAGESIZE)) {
		*(.text)
	}
	.data : ALIGN(CONSTANT(MAXPAGESIZE)) {
		*(.*data)
	}
	.bss : ALIGN(CONSTANT(MAXPAGESIZE)) {
		*(.bss)
	}
	. = 0xfffffffff0000000;
	.realtext : ALIGN(CONSTANT(MAXPAGESIZE)) {
		*(.rtext)
	}
	.realdata : ALIGN(CONSTANT(MAXPAGESIZE)) {
		*(.*rdata)
	}
	.realbss : ALIGN(CONSTANT(MAXPAGESIZE)) {
		*(.rbss)
	}
}
