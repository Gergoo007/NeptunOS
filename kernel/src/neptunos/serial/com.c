#include <neptunos/serial/com.h>

_attr_no_caller_saved_regs void _in8(uint16_t port, uint8_t* data) {
	*data = inb(port);
}
