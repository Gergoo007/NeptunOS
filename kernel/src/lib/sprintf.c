#include <lib/sprintf.h>
#include <serial/serial.h>

void vsprintf(char* out, const char* fmt, va_list args) {
	while (*fmt) {
		if (*fmt == '%') {
			fmt++;
			switch (*fmt) {
				case 's': {
					char* teszt = va_arg(args, char*);
					if (!teszt) {
						u8 len = strlen("(null)")-1;
						char* null = "(null)";

						while (len--) {
							*out = *null;
							out++;
							null++;
						}
						break;
					}
					u8 len = strlen(teszt)-1;
					
					while (len--) {
						*out = *teszt;
						out++;
						teszt++;
					}
					break;
				}
				case 'd': {
					char _tmp[256];
					char* tmp = _tmp;
					int_to_str(va_arg(args, i64), tmp);
					u8 len = strlen(tmp)-1;
					
					while (len--) {
						*out = *tmp;
						out++;
						tmp++;
					}
					break;
				}
				case 'x': {
					char _tmp[16];
					char* tmp = _tmp;
					hex_to_str(va_arg(args, u64), tmp);
					u8 len = strlen(tmp)-1;
					
					while (len--) {
						*out = *tmp;
						out++;
						tmp++;
					}
					break;
				}
				case 'p': {
					char _tmp[16];
					char* tmp = _tmp;
					hexn_to_str(va_arg(args, u64), tmp, 16);
					u8 len = 16;

					while (len--) {
						*out = *tmp;
						out++;
						tmp++;
					}
					break;
				}
				case 'c': {
					putc((char) va_arg(args, int));
					break;
				}
				default: {
					if (*fmt >= '0' && *(fmt+1) <= '9') {
						u8 num = 0;
						num += (*fmt - '0') * 10;
						num += (*(fmt+1) - '0');

						fmt += 2;

						switch (*fmt) {
							case 'x': {
								char _tmp[16];
								char* tmp = _tmp;
								hexn_to_str(va_arg(args, u64), tmp, num);
								
								while (num--) {
									*out = *tmp;
									out++;
									tmp++;
								}
								break;
							}
						}
					}
					break;
				}
			}
			fmt++;
		}
		*out = *fmt;
		fmt++;
		out++;
	}

	*out = '\0';
}

void sprintf(char* out, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vsprintf(out, fmt, args);
	va_end(args);
}
