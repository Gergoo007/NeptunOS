#include <neptunos/tables/idt.h>

idtr idt;

void set_offset(uint64_t offset, idt_desc_entry* entry) {
	entry->offset0 = (uint16_t)(offset & 0x000000000000ffff);
	entry->offset1 = (uint16_t)((offset & 0x00000000ffff0000) >> 16);
	entry->offset2 = (uint32_t)((offset & 0xffffffff00000000) >> 32);
}

uint64_t get_offset(idt_desc_entry* entry) {
	register uint64_t offset;
	offset |= (uint64_t)entry->offset0;
	offset |= (uint64_t)entry->offset1 << 16;
	offset |= (uint64_t)entry->offset2 << 32;
	return offset;
}
