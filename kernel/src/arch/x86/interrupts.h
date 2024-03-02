#pragma once

#include <lib/types.h>

#include <apic/apic.h>

#include <serial/com.h>

#include <lib/symlookup.h>

#pragma pack(1)

typedef struct intf intf;

static _attr_saved_regs void irq_kb() {
	printk("INT TRIG!!!!\n");

	// Read scancode, which is sending an EOI as well
	inb(0x60);

	// Send EOI to the LAPIC
	lapic_eoi();
}

static _attr_int void _irq_kb(intf* _) {
	irq_kb();
}

static _attr_saved_regs void irq_pit() {
	pit_tick += 1;

	// Send EOI to the LAPIC
	lapic_eoi();
}

static _attr_int void _irq_pit(intf* _) {
	irq_pit();
}

static _attr_saved_regs void irq_mouse() {
	printk("mouse event\n");

	// Send EOI to the LAPIC
	lapic_eoi();
}

static _attr_int void _irq_mouse(intf* _) {
	irq_kb();
}

extern void exc0();
extern void exc1();
extern void exc2();
extern void exc3();
extern void exc4();
extern void exc5();
extern void exc6();
extern void exc7();
extern void exc8();
extern void exc9();
extern void exc10();
extern void exc11();
extern void exc12();
extern void exc13();
extern void exc14();
extern void exc15();
extern void exc16();
extern void exc17();
extern void exc18();
extern void exc19();
