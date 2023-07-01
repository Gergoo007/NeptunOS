#include <neptunos/serial/com.h>

void io_wait(void) {
	outb(0, 0x80);
}
