OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kmain)

PHDRS {
	headers PT_PHDR PHDRS;
	text PT_LOAD FILEHDR PHDRS;
	data PT_LOAD;
}

SECTIONS {
	. = 0xfffffff800000000;
	. += SIZEOF_HEADERS;
	.text : ALIGN(0x1000) {
		*(.*text*)
	} : text

	.data : ALIGN(0x1000) {
		*(.data)
		*(.rodata)
	} : data

	.bss : ALIGN(0x1000) {
		*(.bss)
	} :data

	/DISCARD/ : {
		*(.note.gnu.property)
		*(.comment)
		*(.eh_frame)
	}
}
