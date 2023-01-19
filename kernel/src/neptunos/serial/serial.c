#include <neptunos/serial/serial.h>
#include <neptunos/serial/com.h>

#include <neptunos/libk/stdint.h>

void serial_init(void) {
	out8(0x3f8 + 1, 0x00);    // Disable all interrupts
   	out8(0x3f8 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   	out8(0x3f8 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   	out8(0x3f8 + 1, 0x00);    //                  (hi byte)
   	out8(0x3f8 + 3, 0x03);    // 8 bits, no parity, one stop bit
   	out8(0x3f8 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   	out8(0x3f8 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
   	out8(0x3f8 + 4, 0x1E);    // Set in loopback mode, test the serial chip
   	out8(0x3f8 + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

	in8(0x3f8 + 0);

	// If serial is not faulty set it in normal operation mode
	// (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
	out8(0x3f8 + 4, 0x0F);

	serial_write(0x3f8, "\n\r");
}

uint8_t send_ready(uint16_t port) {
	return in8(port + 5) & 0x20;
}

void serial_write(uint16_t port, char *restrict msg) {
	do {
		while (send_ready(port) == 0);
		out8(port, *msg);
		//serial_write_c(port, *msg);
	} while (*(msg++) != '\0');
}

void serial_write_c(uint16_t port, char msg) {
	while (send_ready(port) == 0)
	out8(port, msg);
}

void serial_write_d(uint16_t port, int msg) {
	while (send_ready(port) == 0);
	out8(port, '0' + msg);
}
