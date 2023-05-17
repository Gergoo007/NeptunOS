#include <neptunos/tables/interrupts.h>
#include <neptunos/tables/idt.h>

void idt_add_handler(idt_desc_t* idt, void (*handler)(struct interrupt_frame*)) {
	idt_desc_t* entry = &idt[0xe];
	entry->offs0 = (u64)handler & 0xffff;
	entry->offs1 = ((u64)handler >> 16) & 0xffff;
	entry->offs2 = ((u64)handler) >> 32;
	entry->dpl = 0;
	entry->p = 1;
	entry->gate_type = 0xe;
	entry->ist = 0;
	entry->ss = 0x8;
}

void idt_init(void) {
	const u32 idt_size = sizeof(idt_desc_t) * 256;
	// The IDT consists of 256 descriptors
	idt_desc_t* idt = request_pages(idt_size/0x1000);
	memset(idt, 0, idt_size);

	idt_add_handler(idt, double_flt_handler);

	{
		idtr_t idtr;
		idtr.addr = (u64)idt;
		idtr.limit = sizeof(idt_desc_t)*256-1;

		asm volatile("lidt %0" :: "m"(idtr));
	}
}
