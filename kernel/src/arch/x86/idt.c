#include <arch/x86/idt.h>

idt_entry* idt_entries;

void idt_init(void) {
	idt_entries = pmm_alloc_page();
	memset(idt_entries, 0, 256*16);

	idt_create_entry(&idt_entries[0], (u64)exc0, 0, IDT_TYPE_TRAP);
	idt_create_entry(&idt_entries[1], (u64)exc1, 0, IDT_TYPE_TRAP);
	idt_create_entry(&idt_entries[2], (u64)exc2, 0, IDT_TYPE_TRAP);
	idt_create_entry(&idt_entries[3], (u64)exc3, 0, IDT_TYPE_TRAP);
	idt_create_entry(&idt_entries[4], (u64)exc4, 0, IDT_TYPE_TRAP);
	idt_create_entry(&idt_entries[5], (u64)exc5, 0, IDT_TYPE_TRAP);
	idt_create_entry(&idt_entries[6], (u64)exc6, 0, IDT_TYPE_TRAP);
	idt_create_entry(&idt_entries[7], (u64)exc7, 0, IDT_TYPE_TRAP);
	idt_create_entry(&idt_entries[8], (u64)exc8, 0, IDT_TYPE_TRAP);
	idt_create_entry(&idt_entries[9], (u64)exc9, 0, IDT_TYPE_TRAP);
	idt_create_entry(&idt_entries[10], (u64)exc10, 0, IDT_TYPE_TRAP);
	idt_create_entry(&idt_entries[11], (u64)exc11, 0, IDT_TYPE_TRAP);
	idt_create_entry(&idt_entries[12], (u64)exc12, 0, IDT_TYPE_TRAP);
	idt_create_entry(&idt_entries[13], (u64)exc13, 0, IDT_TYPE_TRAP);
	idt_create_entry(&idt_entries[14], (u64)exc14, 0, IDT_TYPE_TRAP);
	idt_create_entry(&idt_entries[15], (u64)exc15, 0, IDT_TYPE_TRAP);
	idt_create_entry(&idt_entries[16], (u64)exc16, 0, IDT_TYPE_TRAP);
	idt_create_entry(&idt_entries[17], (u64)exc17, 0, IDT_TYPE_TRAP);
	idt_create_entry(&idt_entries[18], (u64)exc18, 0, IDT_TYPE_TRAP);
	idt_create_entry(&idt_entries[19], (u64)exc19, 0, IDT_TYPE_TRAP);

	// idt_create_entry(&idt_entries[0x20], (u64)_irq_pit, 0, IDT_TYPE_INT);
	// idt_create_entry(&idt_entries[0x21], (u64)_irq_kb, 0, IDT_TYPE_INT);
	// idt_create_entry(&idt_entries[0x22], (u64)_irq_mouse, 0, IDT_TYPE_INT);

	idt_attach_isr(0x20, _irq_pit);
	idt_attach_isr(0x21, _irq_kb);
	idt_attach_isr(0x22, _irq_mouse);

	idtr _idtr = {
		.base = (u64)idt_entries,
		.limit = 256*16 - 1,
	};

	idt_load(&_idtr);
}

void idt_create_entry(idt_entry* entry, u64 addr, u8 dpl, u8 type) {
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

void idt_attach_isr(u8 vector, void* isr) {
	idt_create_entry(&idt_entries[vector], (u64)isr, 0, IDT_TYPE_INT);
}

u8 idt_reserve_vector(void* isr) {
	for (u8 i = 20; i < 255; i++) {
		if (idt_entries[i].present) continue;

		idt_attach_isr(i, isr);
		return i;
	}
	error("No free vector!");
	return 0xff;
}
