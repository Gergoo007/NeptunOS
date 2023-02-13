OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kmain)

mmio        = 0xfffffffff8000000;              /* these are configurable for level 2 loaders */
fb          = 0xfffffffffc000000;
bootboot    = 0xffffffffffe00000;
environment = 0xffffffffffe01000;

PHDRS {
	boot PT_LOAD;                                /* one single loadable segment */
}

SECTIONS {
	. = 0xffffffffffe02000;
	KSTART = .;
	.text : {
		KEEP(*(.text.boot)) *(.text .text.*)   /* code */
		*(.rodata .rodata.*)                   /* data */
		*(.data .data.*)
	} :boot
	.bss (NOLOAD) : {                          /* bss */
		. = ALIGN(16);
		*(.bss .bss.*)
		*(COMMON)
	} :boot
	KEND = .;

	/DISCARD/ : { *(.eh_frame) *(.comment) }
}
