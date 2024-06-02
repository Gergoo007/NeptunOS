#pragma once

#include <lib/types.h>
#include <apic/apic.h>
#include <apic/pit/pit.h>
#include <serial/com.h>
#include <lib/symlookup.h>
#include <ps2/kbd.h>

#pragma pack(1)

typedef struct intf intf;

void lapic_eoi();

static u8 key_release = 0;

static _attr_saved_regs void irq_kb() {
	u8 scancode = inb(0x60);

	if (scancode == 0xf0) {
		key_release = 1;
		goto end;
	}
	if (key_release) {
		key_release = 0;
		goto end;
	}

	printk("%c", ps2_kbd_convert(scancode));
end:
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
