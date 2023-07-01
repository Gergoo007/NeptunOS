#include <neptunos/interrupts/idt.h>

// Bitmap to keep track of used IDT entries
bitmap_t idt_bm;
idt_desc_t* idt;

void idt_add_entry(idt_desc_t* idt, void (*handler)(struct interrupt_frame*), u8 vector) {
	idt_desc_t* entry = &idt[vector];
	entry->offs0 = (u64)handler & 0xffff;
	entry->offs1 = ((u64)handler >> 16) & 0xffff;
	entry->offs2 = ((u64)handler) >> 32;
	entry->dpl = 0;
	entry->p = 1;
	entry->gate_type = 0xe;
	entry->ist = 0;
	entry->ss = 0x8;

	// Set vector as used
	bm_set(&idt_bm, vector, 1);
}

// Scan for free space inside the IDT, add the isr,
// mark it as used, return the vector
// Basically runtime version of idt_add_entry
u8 idt_add_isr(void (*handler)(struct interrupt_frame*)) {
	// Interrupts before 0x20 are reserved
	u8 free_vec = bm_find_clear(&idt_bm, 0x20);
	printk("IDT: %p\n", idt);
	idt_add_entry(idt, handler, free_vec);

	// Reload IDT
	idtr_t idtr;
	idtr.addr = (u64)idt;
	idtr.limit = sizeof(idt_desc_t)*256-1;
	asm volatile("lidt %0" :: "m"(idtr));

	return free_vec;
}

void idt_init(void) {
	const u32 idt_size = sizeof(idt_desc_t) * 256;
	// The IDT consists of 256 descriptors
	idt = request_pages(idt_size/0x1000);
	map_page(idt, idt, MAP_FLAGS_IO_DEFAULTS | MAP_FLAG_WRITE_THROUGH);
	memset(idt, 0, idt_size);

	// Initialize bitmap
	idt_bm.address = request_page();
	// We need to keep track of 256 entries
	idt_bm.size = 256 / 8 + 1;

	memset(idt_bm.address, 0, idt_bm.size);

	for (u8 i = 0; 1; i++) {
		if(exception_isrs[i][0] == 0)
			break;
		
		idt_add_entry(idt, (void (*)(struct interrupt_frame *)) exception_isrs[i][0], exception_isrs[i][1]);
	}
	
	idtr_t idtr;
	idtr.addr = (u64)idt;
	idtr.limit = sizeof(idt_desc_t)*256-1;

	asm volatile("lidt %0" :: "m"(idtr));
}
