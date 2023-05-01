/* Credit for the higher half theory: https://github.com/thomasloven/mittos64 */

OUTPUT_FORMAT(elf64-x86-64)
ENTRY(onboot)

/* kernel will get loaded at +1M */
KERNEL_INITAL = 1M;

SECTIONS {
	.multiboot : ALIGN(8) {
		*(.multiboot)
	}

	. = KERNEL_INITAL;

	.text ALIGN(4K) : AT(ADDR(.text)) {
		*(.text)
	}
	.data ALIGN(4K) : AT(ADDR(.data)) {
		*(.*data)
	}
	.bss ALIGN(4K) : AT(ADDR(.bss)) {
		*(.bss)
	}

	/*. = 0xffffffffffd00000;*/
}
