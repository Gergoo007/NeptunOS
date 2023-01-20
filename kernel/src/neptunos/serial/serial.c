#include <neptunos/serial/serial.h>
#include <neptunos/serial/com.h>

#include <neptunos/libk/stdint.h>

void serial_init(void) {
	outb(0x00, 0x3f8 + 1);    // Disable all interrupts
   	outb(0x80, 0x3f8 + 3);    // Enable DLAB (set baud rate divisor)
   	outb(0x03, 0x3f8 + 0);    // Set divisor to 3 (lo byte) 38400 baud
   	outb(0x00, 0x3f8 + 1);    //                  (hi byte)
   	outb(0x03, 0x3f8 + 3);    // 8 bits, no parity, one stop bit
   	outb(0xC7, 0x3f8 + 2);    // Enable FIFO, clear them, with 14-byte threshold
   	outb(0x0B, 0x3f8 + 4);    // IRQs enabled, RTS/DSR set
   	outb(0x1E, 0x3f8 + 4);    // Set in loopback mode, test the serial chip
   	outb(0xAE, 0x3f8 + 0);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

	inb(0x3f8 + 0);

	// If serial is not faulty set it in normal operation mode
	// (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
	outb(0x0F, 0x3f8 + 4);

	serial_write(0x3f8, "\n\r");
}

uint8_t send_ready(uint16_t port) {
	return inb(port + 5) & 0x20;
}

void serial_write(uint16_t port, char *restrict msg) {
	do {
		while (send_ready(port) == 0);
		outb(*msg, port);
		//serial_write_c(port, *msg);
	} while (*(msg++) != '\0');
}

void serial_write_c(uint16_t port, char msg) {
	while (send_ready(port) == 0)
	outb(msg, port);
}

void serial_write_d(uint16_t port, int msg) {
	while (send_ready(port) == 0);
	outb('0' + msg, port);
}
