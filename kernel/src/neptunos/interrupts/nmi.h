#pragma once

#include <neptunos/serial/com.h>

inline static void nmi_enable(void) {
	outb(0x70, inb(0x70) & 0x7F);
	inb(0x71);
}

inline static void nmi_disable(void) {
	outb(0x70, inb(0x70) | 0x80);
	inb(0x71);
}
