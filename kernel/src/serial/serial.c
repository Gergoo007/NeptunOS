#include <serial/serial.h>

void sprintk(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char buffer[128];
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

// LF -> CRLF
void puts_translate(const char* s) {
	while (*s) {
		if (*s == '\n')
			puts("\n\r");
		else
			putc(*s);
		s++;
	}
}
