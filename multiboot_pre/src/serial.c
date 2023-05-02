#include "serial.h"

void debug_putc(const char c) {
	__asm__ volatile("outb %0, %1" :: "a"(c), "d"(0x3f8));
}

void debug_puts(const char* str) {
	while (*str != '\0')
		debug_putc(*str++);
}

void printk(char* fmt, ...) {
	va_list arg_list;
	va_start(arg_list, fmt);
	while(*fmt != '\0') {
		if (*fmt == '%') {
			fmt++;
			switch (*fmt) {
				case 'c':
					debug_putc((char) va_arg(arg_list, uint32_t));
					break;
				case 'd': {
					char res[256] = "";
					int64_to_str(va_arg(arg_list, int64_t), res, 10);
					debug_puts(res);
					break;
				}
				case 'u':
					fmt++;
					switch(*fmt) {
						case 'd': {
							char res[256] = "";
							debug_puts(uint_to_str(va_arg(arg_list, uint64_t), res, 10));
							break;
						}
					}
					break;
				case 's':
					printk(va_arg(arg_list, char*));
					break;
				case 'p': {
					char testbuf[17];
					uint_to_str((uint64_t)va_arg(arg_list, void*), testbuf, 16);
					debug_puts(testbuf);
					break;
				}
				case 'x': {
					char buffer[256];
					memset(buffer, 0, 255);
					uint_to_str((uint64_t)va_arg(arg_list, uint64_t), buffer, 16);
					debug_puts(buffer);
					break;
				}
				case 'X': {
					char buffer[256];
					memset(buffer, 0, 255);
					uint_to_str((uint64_t)va_arg(arg_list, uint64_t), buffer, 16);
					debug_puts(buffer);
					break;
				}
				default: {
					if (*fmt <= '9' && *fmt >= '0') {
						uint8_t first = (uint8_t)(*fmt++) - (uint8_t)'0';
						uint8_t second = (uint8_t)*(fmt++) - (uint8_t)'0';
						uint8_t final = second + (first * 10);

						switch (*fmt) {
							case 'x': {
								char buffer[final + 1];
								memset(buffer, 0, final + 1);
								uint_to_str((uint64_t)va_arg(arg_list, uint64_t), buffer, 16);
								debug_puts(buffer);
								break;
							}
							case 'X': {
								char buffer[final + 1];
								memset(buffer, 0, final + 1);
								uint_to_str((uint64_t)va_arg(arg_list, uint64_t), buffer, 16);
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
								uint64_t num = va_arg(arg_list, uint64_t);
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