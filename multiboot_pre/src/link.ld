OUTPUT_FORMAT(elf64-x86-64)
ENTRY(onboot)

SECTIONS {
	.multiboot : ALIGN(8) {
		*(.multiboot)
	}

	. = 1M;

	.text ALIGN(4K) : AT(ADDR(.text)) {
		*(.text)
	}
	.data ALIGN(4K) : AT(ADDR(.data)) {
		*(.*data)
	}
	.bss ALIGN(4K) : AT(ADDR(.bss)) {
		*(.bss)
	}
}
