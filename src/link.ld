OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kmain)

SECTIONS {
	. = 0xFFFFFF8000000000;

	.text ALIGN(4K) : AT(ADDR(.text)) {
		*(.text)
	}
	.data ALIGN(4K) : AT(ADDR(.data)) {
		*(.*data)
	}
	.rodata ALIGN(4K) : AT(ADDR(.rodata)) {
		*(.*rodata)
	}
	.bss ALIGN(4K) : AT(ADDR(.bss)) {
		*(.bss)
	}
}
