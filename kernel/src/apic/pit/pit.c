#include <apic/pit/pit.h>

u64 pit_tick = 0;

void sleep(u32 time) {
	u64 start = pit_tick;
	while (pit_tick < start+time) {
		asm volatile ("hlt");
	}
}

void pit_init() {
	clearint();

	pit_cmd_reg cmd = {
		.bcd = 0,
		.operation = PIT_RATE_GEN,
		.access = PIT_LOHIBYTE,
		.ch = 0,
	};

	outb(cmd.raw, PIT_CMD);

	// One tick takes approximately 1 ms
	u16 count = 1250;

	outb(count & 0x00ff, PIT_CH0);
	outb((count & 0xff00) >> 8, PIT_CH0);

	setint();
}
