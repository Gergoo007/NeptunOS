ENTRY("mod_main")

SECTIONS {
	. = SIZEOF_HEADERS;

	. = ALIGN(0x200000);
	.text : {
		*(.text)
	}

	. = ALIGN(0x200000);
	.data : {
		*(.data)
	}

	. = ALIGN(0x200000);
	.bss : {
		*(.bss)
	}

	/DISCARD/ : {
		*(.note.gnu.property)
		*(.comment)
		*(.eh_frame)

		*(.dyn*)
		*(.interp)
		*(.*hash*)
	}
}
