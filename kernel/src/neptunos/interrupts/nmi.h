#pragma once

#include <neptunos/serial/com.h>

inline static void nmi_enable(void) {
	io_wait(2);
	u8 data = inb(0x70) & 0b01111111;
	io_wait(2);
	outb(data, 0x70);
	io_wait(2);
	inb(0x71);
	io_wait(2);
}

inline static void nmi_disable(void) {
	io_wait(2);
	u8 data = inb(0x70) | 0b10000000;
	io_wait(2);
	outb(data, 0x70);
	io_wait(2);
	inb(0x71);
	io_wait(2);
}
