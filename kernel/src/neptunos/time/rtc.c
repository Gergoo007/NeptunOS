#include "rtc.h"

void rtc_init(void) {
	nmi_disable();
	asm("cli");

	// outb(0x70, 0x8B);
	// u8 prev = inb(0x71);
	// outb(0x70, 0x8B);
	// outb(0x71, prev | 0x40);

	asm("sti");
	nmi_enable();
}
