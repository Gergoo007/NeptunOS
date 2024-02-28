#pragma once

#include <lib/attrs.h>

#include <graphics/panic.h>

#include <apic/apic.h>
#include <apic/apic.h>

#include <serial/com.h>

#include <lib/symlookup.h>

typedef struct int_frame {
	u64 err;
	u64 rip;
	u64 cs;
	u64 rflags;
	u64 rsp;
	u64 ss;
} int_frame_t;

static void print_details(int_frame_t* int_frame) {
	printk(
		"RIP: %p CS: %p\nRFLAGS: %p RSP: %p\nSS: %p\n",
		int_frame->rip, int_frame->cs, int_frame->rflags, int_frame->rsp, int_frame->ss
	);

	// char buf[1024];

	// u64 rbp;
	// asm volatile ("movq %%rbp, %0" : "=r"(rbp));

	// stacktrace((stackframe_t*)rbp, buf);

	// printk("%s", buf);
}

static _attr_saved_regs void invalid_opcode(int_frame_t* int_frame) {
	printk("Invalid Opcode found, halting!\n");
	print_details(int_frame);
	asm volatile ("cli");
	asm volatile ("hlt");
}

static _attr_saved_regs void double_fault(int_frame_t* int_frame) {
	printk("Double Fault encountered, halting!\n");
	asm volatile ("cli");
	asm volatile ("hlt");
}

static _attr_saved_regs void page_fault(int_frame_t* int_frame) {
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

static _attr_saved_regs void gp_fault(int_frame_t* int_frame) {
	printk("General Protection Fault [%p]\n", int_frame->err);
	print_details(int_frame);

	asm volatile ("cli");
	asm volatile ("hlt");
}

static _attr_int void _0x6(int_frame_t* int_frame) {
	invalid_opcode(int_frame);
}

static _attr_int void _0x8(int_frame_t* int_frame) {
	double_fault(int_frame);
}

static _attr_int void _0xe(int_frame_t* int_frame) {
	page_fault(int_frame);
}

static _attr_int void _0xd(int_frame_t* int_frame) {
	gp_fault(int_frame);
}

static _attr_saved_regs void irq_kb(int_frame_t* int_frame) {
	printk("INT TRIG!!!!\n");
			
	// Read scancode, which is sending an EOI as well
	inb(0x60);
			
	// Send EOI to the LAPIC
	lapic_write_reg(0xb0, 0);
}

static _attr_int void _irq_kb(int_frame_t* int_frame) {
	irq_kb(int_frame);
}

static _attr_saved_regs void irq_pit(int_frame_t* int_frame) {
	pit_tick += 1;

	// Send EOI to the LAPIC
	lapic_write_reg(0xb0, 0);
}

static _attr_int void _irq_pit(int_frame_t* int_frame) {
	irq_pit(int_frame);
}

static _attr_saved_regs void irq_mouse(int_frame_t* int_frame) {
	printk("mouse event\n");

	// Send EOI to the LAPIC
	lapic_write_reg(0xb0, 0);
}

static _attr_int void _irq_mouse(int_frame_t* int_frame) {
	irq_kb(int_frame);
}
