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

void _debug_putc(const char c) {
	__asm__ volatile("outb %0, %1" :: "a"(c), "d"(0x3f8));
}

void _debug_puts(const char* str) {
	while (*str != '\0')
		debug_putc(*str++);
}

void _serprintk(char* fmt, ...) {
	va_list arg_list;
	va_start(arg_list, fmt);
	while(*fmt != '\0') {
		if (*fmt == '%') {
			fmt++;
			switch (*fmt) {
				case 'c':
					debug_putc((char) va_arg(arg_list, u32));
					break;
				case 'd': {
					char res[256] = "";
					int64_to_str(va_arg(arg_list, i64), res, 10);
					debug_puts(res);
					break;
				}
				case 'u':
					fmt++;
					switch(*fmt) {
						case 'd': {
							char res[256] = "";
							debug_puts(uint_to_str(va_arg(arg_list, u64), res, 10));
							break;
						}
					}
					break;
				case 's':
					printk(va_arg(arg_list, char*));
					break;
				case 'p': {
					char testbuf[17];
					uint_to_str((u64)va_arg(arg_list, void*), testbuf, 16);
					debug_puts(testbuf);
					break;
				}
				case 'x': {
					char buffer[256];
					memset(buffer, 0, 255);
					uint_to_str((u64)va_arg(arg_list, u64), buffer, 16);
					debug_puts(buffer);
					break;
				}
				case 'X': {
					char buffer[256];
					memset(buffer, 0, 255);
					uint_to_str((u64)va_arg(arg_list, u64), buffer, 16);
					debug_puts(buffer);
					break;
				}
				default: {
					if (*fmt <= '9' && *fmt >= '0') {
						u8 first = (u8)(*fmt++) - (u8)'0';
						u8 second = (u8)*(fmt++) - (u8)'0';
						u8 final = second + (first * 10);

						switch (*fmt) {
							case 'x': {
								char buffer[final + 1];
								memset(buffer, 0, final + 1);
								uint_to_str((u64)va_arg(arg_list, u64), buffer, 16);
								debug_puts(buffer);
								break;
							}
							case 'X': {
								char buffer[final + 1];
								memset(buffer, 0, final + 1);
								uint_to_str((u64)va_arg(arg_list, u64), buffer, 16);
								debug_puts(buffer);
								break;
							}
							case 's': {
								char buffer[final + 1];
								memcpy(buffer, (void*)va_arg(arg_list, char*), final);
								buffer[final] = '\0';
								debug_puts(buffer);
								break;
							}
							case 'b': {
								u64 num = va_arg(arg_list, u64);
								while (final-- > 0) {
										if (num & (1 << final))
											debug_putc('1');
										else
											debug_putc('0');
								}
								break;
							};
							default:
								debug_puts("Invalid format: ");
								debug_putc(*fmt);
								debug_puts("!\n");
								break;
						}
					}
				}
				break;
			}
		} else {
			debug_putc(*fmt);
		}

		fmt++;
	}
	va_end(arg_list);
}
