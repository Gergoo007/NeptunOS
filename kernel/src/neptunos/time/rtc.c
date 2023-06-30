#include "rtc.h"

void enable_rtc(void) {
	nmi_disable();
	asm("cli");

	// Set rate so that it sends an interrupt every 125 ms
	outb(0x8A, 0x70);
	u8 prev = inb(0x71);
	outb(0x8A, 0x70);
	outb((prev & 0xF0) | 0b00001101, 0x71);

	outb(0x8A, 0x70);
	prev = inb(0x71);
	printk("A register: %d\n", prev & 0x0f);
	outb(0x8B, 0x70);
	prev = inb(0x71);
	outb(0x8B, 0x70);
	outb(prev | 0x40, 0x71);

	asm("sti");
	nmi_enable();
}
