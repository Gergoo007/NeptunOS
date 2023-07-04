#include "rtc.h"

// Only works 1/5th of the time on real hardware for some reason
void enable_rtc(void) {
	nmi_disable();

	rtc_reg_a_t reg_a;
	rtc_reg_b_t reg_b;

	outb(0x0A, 0x70);
	reg_a.val = inb(0x71);
	outb(0x0B, 0x70);
	reg_b.val = inb(0x71);

	reg_a.rs = 0b1101;
	reg_a.dv = 0b010;
	
	reg_b.sqw_enable = 0;
	reg_b.int_on_update_end = 0;
	reg_b.alarm_int = 0;
	reg_b.periodic_int = 1;
	reg_b.set = 0;

	outb(0x0A, 0x70);
	outb(reg_a.val, 0x71);

	outb(0x0B, 0x70);
	outb(reg_b.val, 0x71);

	outb(0x0C, 0x70);
	inb(0x71);

	nmi_enable();
}
