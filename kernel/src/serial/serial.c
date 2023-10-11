#include <serial/serial.h>

// Egy 2 KiB-os buffer amibe az ideiglenes sprintf
// kimenet megy
char buffer[2048] = "";

void sprintk(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	vsprintf(buffer, fmt, args);
	puts(buffer);

	va_end(args);
}

void putc(char c) {
	asm volatile ("outb %0, %1" :: "a"(c), "d"(0x3f8));
}

void puts(const char* s) {
	while (*s) {
		putc(*s);
		s++;
	}
}
