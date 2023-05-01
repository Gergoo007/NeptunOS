/* Credit for the higher half theory: https://github.com/thomasloven/mittos64 */

OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kmain)

/* To be loaded here */
KERNEL_OFFSET = 0xFFFFFF8000000000;

SECTIONS {
	. = KERNEL_OFFSET;
	
	.text ALIGN(4K) : AT(ADDR(.text)) {
		*(.text)
	}
	.data ALIGN(4K) : AT(ADDR(.data)) {
		*(.data)
		*(.rodata)
	}
	.bss ALIGN(4K) : AT(ADDR(.bss)) {
		*(.bss)
	}

	/*. = 0xffffffffffd00000;*/
}
