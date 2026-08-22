ENTRY("kmain")

PHDRS {
	boot PT_LOAD;
	text PT_LOAD;
	rodata PT_LOAD;
	data PT_LOAD;
}

SECTIONS {
	. = 0xffffffff80000000;

	IMAGE_START = .;
	.limine_requests : {
		KEEP(*(.limine_requests_start))
		KEEP(*(.limine_requests))
		KEEP(*(.limine_requests_end))
	} :boot

	. = ALIGN(CONSTANT(MAXPAGESIZE));
	.text : {
		*(.text .text.*)
	} :text

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.rodata : {
		*(.rodata .rodata.*)
	} :rodata

	.note.gnu.build-id : {
		*(.note.gnu.build-id)
	} :rodata

	.init_array : {
		__init_array = .;
		KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
		KEEP(*(.init_array .ctors))
		__init_array_end = .;
	} :rodata

	. = ALIGN(CONSTANT(MAXPAGESIZE));

	.data : {
		*(.data .data.*)
	} :data

	.bss : {
		*(.bss .bss.*)
		*(COMMON)
	} :data
	IMAGE_END = .;

	/DISCARD/ : {
		*(.eh_frame*)
		*(.note .note.*)
	}
}
