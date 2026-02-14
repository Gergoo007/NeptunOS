#include "terminal.hh"

void puts(const char* s) {
	asm volatile ("syscall" :: "a"(1), "d"(s));
}
