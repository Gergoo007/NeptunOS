#include "pit.h"

void pit_set_mode(pit_mode_t* mode) {
	outb(mode->val, PIT_CMD);
}

void pit_set_reload_val(u16 reload_val) {
	outb(reload_val & 0x00ff, PIT_CH0);
	outb((reload_val & 0xff00) >> 8, PIT_CH0);
}

// If I set the mode to rate gen, and the reload value to 59659,
// the PIT will generate IRQs at a frequency of 20 Hz,
// triggering interrupts every 50 milliseconds
void pit_init(void) {
	pit_mode_t mode;
	mode.access_mode = PIT_ACCESS_LOHI_BYTE;
	mode.op_mode = PIT_MODE_RATE_GEN;
	mode.bcd = 0;
	mode.ch = 0;

	// Set rate generator mode
	pit_set_mode(&mode);

	// Set reload value to start the timer
	pit_set_reload_val(59659);
	
}
