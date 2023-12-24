#include <arch/x86/isrs.h>

_attr_saved_regs void invalid_opcode(int_frame_t* int_frame) {
	printk("Invalid Opcode found, halting!\n");
	asm volatile ("cli");
	asm volatile ("hlt");
}

_attr_saved_regs void double_fault(int_frame_t* int_frame) {
	printk("Double Fault encountered, halting!\n");
	asm volatile ("cli");
	asm volatile ("hlt");
}

_attr_saved_regs void page_fault(int_frame_t* int_frame) {
	u64 addr;
	asm volatile ("movq %%cr2, %0" : "=a"(addr));

	// printk igénybe veszi a heapet, ezzel a módszerrel csak a stack kell
	char message[64];
	sprintf(message, "Page Fault at %p encountered, halting!", (void*)addr);
	puts(message);
	kputs(message);
	
	asm volatile ("cli");
	asm volatile ("hlt");
}

_attr_saved_regs void gp_fault(int_frame_t* int_frame) {
	printk("General Protection Fault encountered, halting!\n");
	asm volatile ("cli");
	asm volatile ("hlt");
}

_attr_int void _0x6(int_frame_t* int_frame) {
	invalid_opcode(int_frame);
}

_attr_int void _0x8(int_frame_t* int_frame) {
	double_fault(int_frame);
}

_attr_int void _0xe(int_frame_t* int_frame) {
	page_fault(int_frame);
}

_attr_int void _0xd(int_frame_t* int_frame) {
	gp_fault(int_frame);
}
