#include <arch/x86/idt.h>

void idt_init(void) {
	idt_entry_t* entries = pmm_alloc_page();
	memset(entries, 0, 256*16);

	idt_create_entry(&entries[0x6], (u64)_0x6, 0, IDT_TYPE_TRAP);
	idt_create_entry(&entries[0x8], (u64)_0x8, 0, IDT_TYPE_TRAP);
	idt_create_entry(&entries[0xe], (u64)_0xe, 0, IDT_TYPE_TRAP);
	idt_create_entry(&entries[0xd], (u64)_0xd, 0, IDT_TYPE_TRAP);

	idt_create_entry(&entries[0x20], (u64)_irq_pit, 0, IDT_TYPE_INT);
	idt_create_entry(&entries[0x21], (u64)_irq_kb, 0, IDT_TYPE_INT);
	idt_create_entry(&entries[0x22], (u64)_irq_mouse, 0, IDT_TYPE_INT);

	idtr_t idtr = {
		.base = (u64)entries,
		.limit = 256*16 - 1,
	};

	idt_load(&idtr);
}

void idt_create_entry(idt_entry_t* entry, u64 addr, u8 dpl, u8 type) {
	entry->gate_type = type;
	entry->dpl = dpl;
	entry->ist = 0;
	entry->present = 1;

	if (dpl == 0)
		entry->ss = 0x08; // kcode
	else
		entry->ss = 0x20; // ucode

	entry->base0 =	addr & 0x000000000000ffff;
	entry->base1 = (addr & 0x00000000ffff0000) >> 16;
	entry->base2 = (addr & 0xffffffff00000000) >> 32;
}
