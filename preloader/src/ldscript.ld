OUTPUT_FORMAT(elf64-x86-64)
ENTRY(pmain)

SECTIONS {
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

	/DISCARD/ : {
		*(.note.gnu.property)
		*(.comment)
		*(.eh_frame)
	}
}
