#include <serial.h>
#include <strings.h>

void putc(char c) {
	asm volatile("outb %0, %1" :: "a"(c), "d"(0x3f8));
}

void puts(const char* restrict s) {
	while(*s != '\0')
		putc(*(s++));
}

// TODO: negatív számok
void int_to_str(i64 i, char* str) {
	u8 negative = (i < 0) ? 1 : 0;
	u8 len = 1;

	if (negative)
		i *= -1;

	// Ha nulla, akkor egyszerű a dolgunk
	// Ugyanakkor ez bypassel egy buggot:
	// mivel a szám 0, a length is 0 lesz, ezért üres stringet kapunk
	if (!i) {
		str[0] = '0';
		str[1] = '\0';
		return;
	}

	// Számokat át konvertáljuk karakterekké
	for (u8 j = 0; i > 0; j++, i /= 10) {
		str[j] = (i % 10) + '0';
		len++;
	}

	// Stringet megfordítjuk
	u8 cycles = len / 2;
	for (u8 j = 0; j < cycles; j++) {
		char tmp = str[j];
		str[j] = str[len-j-2];
		str[len-j-2] = tmp;
	}

	str[len-1] = '\0';

	if (negative) {
		// A kötőjelet oda kell rakni ha negatív
		char tmp[256];
		tmp[0] = '-';
		strcpy(str, tmp+1);
		strcpy(tmp, str);
	}
}

void uint_to_str(u64 i, char* str) {
	u8 len = 1;

	// Számokat át konvertáljuk karakterekké
	for (u8 j = 0; i > 0; j++, i /= 10) {
		str[j] = (i % 10) + '0';
		len++;
	}

	// Stringet megfordítjuk
	u8 cycles = len / 2;
	for (u8 j = 0; j < cycles; j++) {
		char tmp = str[j];
		str[j] = str[len-j-2];
		str[len-j-2] = tmp;
	}

	str[len-1] = '\0';
}

void hex_to_str(u64 i, char* str) {
	const char* numbers = "0123456789abcdef";
	u8 len = 1;

	// Számokat át konvertáljuk karakterekké
	for (u8 j = 0; i > 0; j++, i /= 16) {
		str[j] = *((i % 16) + numbers);
		len++;
	}

	// Stringet megfordítjuk
	u8 cycles = len / 2;
	for (u8 j = 0; j < cycles; j++) {
		char tmp = str[j];
		str[j] = str[len-j-2];
		str[len-j-2] = tmp;
	}

	str[len-1] = '\0';
}

void ptr64_to_str(u64 i, char* str) {
	const char* numbers = "0123456789abcdef";
	u8 len = 1;

	// Számokat át konvertáljuk karakterekké
	for (u8 j = 0; j < 16; j++, i /= 16) {
		str[j] = *((i % 16) + numbers);
		len++;
	}

	// Stringet megfordítjuk
	u8 cycles = len / 2;
	for (u8 j = 0; j < cycles; j++) {
		char tmp = str[j];
		str[j] = str[len-j-2];
		str[len-j-2] = tmp;
	}

	str[len-1] = '\0';
}

void printf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	
	while (*fmt != 0) {
		if (*fmt == '%') {
			fmt++;
			switch (*fmt) {
				case 's': {
					printf(fmt);
					break;
				}
				case 'd': {
					char buf[256];
					int_to_str(va_arg(args, i64), buf);
					puts(buf);
					break;
				}
				case 'x': {
					char buf[256];
					hex_to_str(va_arg(args, u64), buf);
					puts(buf);
					break;
				}
				case 'p': {
					char buf[16];
					ptr64_to_str(va_arg(args, u64), buf);
					puts(buf);
					break;
				}
				case 'c': {
					putc((char) va_arg(args, int));
					break;
				}
				default: {
					puts("<<ERVENYTELEN FORMATUM>>");
					break;
				}
			}
			fmt++;
		} else {
			putc(*fmt);
			fmt++;
		}
	}

	va_end(args);
}
