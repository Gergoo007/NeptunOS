ENTRY("kmain")

SECTIONS {
	. = 0xffffffffce000000;
	. += SIZEOF_HEADERS;
	. = ALIGN(0x1000);

	IMAGE_START = .;
	IMAGE_START = .;
	.text : {
		*(.text)
		*(.rodata)
	} :text

	.data : {
		*(.data)
	} :data

	.bss : {
		*(.bss)
	} :data
	IMAGE_END = .;

	/DISCARD/ : {
		*(.note.gnu.property)
		*(.comment)
		*(.eh_frame)
	}
}
