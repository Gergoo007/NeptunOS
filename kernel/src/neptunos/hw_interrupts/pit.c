#include <neptunos/hw_interrupts/pit.h>
#include <neptunos/libk/string.h>
#include <neptunos/serial/com.h>

// With the default speed, it would need about 5 845 420 461 years to overflow
uint64_t counter = 0;

uint64_t base_freq = 1193182;
uint16_t divisor = 65535;

void pit_configure_channel(uint8_t ch) {
	pit_command_t command;
	uint8_t cmd;
	memset(&command, 0, 1);

	command.channel = PIT_CH_0;
	command.access_mode = PIT_LOHIBYTE;
	command.oper_mode = PIT_SQUARE_WAVE_GEN;
	command.bcd_mode = 0;

	memcpy(&cmd, &command, 1);

	_out8(PIT_IO_MODE_CMD, cmd);

	asm("cli");

	uint32_t count = 1000;
	_out8(PIT_IO_CH_0, count & 0x00FF); // Send low bits
	_out8(PIT_IO_CH_0, (count & 0xFF00) >> 8); // Send high bits
}


void pit_delay(uint64_t milliseconds) {
	uint64_t start = counter;
	while (counter < start + (milliseconds / 10)) {
		asm("hlt");
		// For some reason this is needed
		printk("");
	}
}

void pit_set_divisor(uint16_t div) {
	if (div < 100) div = 100;
	divisor = div;
	_out8(0x40, (uint8_t)(divisor & 0x00ff));
	_out8(0x40, (uint8_t)((divisor & 0xff00) >> 8));
}

uint64_t pit_get_frequency() {
	return base_freq / divisor;
}

void pit_set_frequency(uint64_t frequency) {
	pit_set_divisor(base_freq / frequency);
}

void pit_tick() {
	counter++;
	
}
