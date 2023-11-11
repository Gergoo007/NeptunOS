OUTPUT_FORMAT(elf64-x86-64)
ENTRY(pmain)

SECTIONS {
	PRELOADER_START = .;
	.text : {
		*(.*text*)
	}

	.data : {
		*(.data)
		*(.rodata)
	}

	.bss : {
		*(.bss)
	}
	PRELOADER_END = .;

	/DISCARD/ : {
		*(.note.gnu.property)
		*(.comment)
		*(.eh_frame)
	}
}
