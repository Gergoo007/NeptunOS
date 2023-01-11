OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kmain)

SECTIONS {
	/* TODO: prepare bootloader and load it in higher half */
	. = 0x4000000; /* load kernel at +64M */

	KSTART = .;
	
	.text : ALIGN(0x1000) {
		*(.text)
	}
	
	.data : ALIGN(0x1000) {
		*(.data .data.*)
	}
	
	.rodata : ALIGN(0x1000) {
		*(.rodata .rodata.*)
	}
	
	.bss (NOLOAD) : ALIGN(0x1000) {
		*(.bss .bss.*)
		*(COMMON)
	}
	
	KEND = .;

	/DISCARD/ : { *(.eh_frame) *(.comment) }
}
