OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kmain)

PHDRS {
	text PT_LOAD FILEHDR PHDRS;
	data PT_LOAD;
}

SECTIONS {
	/* set address, otherwise it might load at an address used by UEFI/bootloader */
	. = 0x01000000;

	KSTART = .;
	
	.text : {
		*(.text)
	} :text
	
	.data : {
		*(.data .data.*)
	} :data
	
	.rodata : {
		*(.rodata .rodata.*)
	} :data
	
	.bss (NOLOAD) : {
		*(.bss .bss.*)
		*(COMMON)
	} :data
	
	KEND = .;

	/DISCARD/ : { *(.eh_frame) *(.comment) }
}
