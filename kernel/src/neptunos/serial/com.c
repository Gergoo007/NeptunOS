#include <neptunos/serial/com.h>

void io_wait(u32 rounds) {
	while(rounds--)
		outb(0, 0x80);
}
