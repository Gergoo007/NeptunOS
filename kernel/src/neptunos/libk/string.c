#include <neptunos/libk/string.h>

#include <neptunos/libk/stdint.h>
#include <neptunos/libk/stdarg.h>
#include <neptunos/libk/limits.h>
#include <neptunos/libk/stdint.h>
#include <neptunos/graphics/text_renderer.h>

void* (*memcpy)(void* dest, const void* src, size_t n) = NULL;

int str_to_int(const char* str, int* num) {
	int sign = 1, base = 0, i = 0;

	// if whitespaces then ignore.
	while (str[i] == ' ')
	{
		i++;
	}

	// sign of number
	if (str[i] == '-' || str[i] == '+')
	{
		sign = 1 - 2 * (str[i++] == '-');
	}

	// checking for valid input
	while (str[i] >= '0' && str[i] <= '9')
	{
		// handling overflow test case
		if (base > INT_MAX / 10
			|| (base == INT_MAX / 10
			&& str[i] - '0' > 7))
		{
			if (sign == 1)
				return INT_MAX;
			else
				return INT_MIN;
		}
		base = 10 * base + (str[i++] - '0');
	}

	*num = base * sign;
	return base * sign;
}

char* int_to_str(int value, char* result, int base) {
	// check that the base if valid
	if (base < 2 || base > 36) { *result = '\0'; return result; }

	char* ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while ( value );

	// Apply negative sign
	if (tmp_value < 0) *ptr++ = '-';
	*ptr-- = '\0';
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}

char* int64_to_str(int64_t value, char* result, int base) {
	// check that the base if valid
	if (base < 2 || base > 36) { *result = '\0'; return result; }

	char* ptr = result, *ptr1 = result, tmp_char;
	int64_t tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while ( value );

	// Apply negative sign
	if (tmp_value < 0) *ptr++ = '-';
	*ptr-- = '\0';
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}

char* uint_to_str(uint64_t value, char* result, int base) {
	// check that the base if valid
	if (base < 2 || base > 36) { *result = '\0'; return result; }

	char* ptr = result, *ptr1 = result, tmp_char;
	uint64_t tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while ( value );

	// Apply negative sign
	if (tmp_value < 0) *ptr++ = '-';
	*ptr-- = '\0';
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}

// void* memcpy(void* dest, const void* src, size_t n) {
// 	for (; n; n--)
// 		*((uint8_t*)dest++) = *((uint8_t*)src++);
	
// 	return dest;
// }

// void* memset(void* dest, const int src, size_t n) {
// 	for (; n; n--, dest++)
// 		*((uint32_t*)dest) = src;

// 	return dest;
// }

uint8_t strncmp(char* str1, char* str2, uint16_t len) {
	uint8_t result = 0;
	while((len--) > 0) {
		result |= (str1[len] ^ str2[len]);
	}
	return !result;
}

u16 strlen(const char* restrict str) {
	u16 res = 0;
	while(*(str+res) != '\0')
		res++;
	return res;
}

void sprintf(char* dest, const char *restrict fmt, ...) {
	va_list arg_list;
	va_start(arg_list, fmt);
	for (u16 fmt_c = 0, dest_c = 0; ; fmt_c++, dest_c++) {
		if (fmt[fmt_c] == '%') {
			fmt_c++;
			switch (fmt[fmt_c]) {
				case 'c':
					dest[fmt_c] = (char) va_arg(arg_list, u32);
					dest++;
					break;
				case 'd': {
					char buf[256];
					int64_to_str(va_arg(arg_list, i64), buf, 10);
					memcpy(dest+dest_c, buf, strlen(buf));
					dest_c += strlen(buf)-1;
					break;
				}
				case 'u':
					switch(fmt[fmt_c]) {
						case 'd': {
							char buf[256];
							uint_to_str(va_arg(arg_list, u64), buf, 10);
							memcpy(dest+dest_c, buf, strlen(buf));
							dest_c += strlen(buf)-1;
							break;
						}
					}
					break;
				case 's': {
					const char* arg = va_arg(arg_list, char*);
					memcpy(dest+dest_c, arg, strlen(arg));
					dest_c += strlen(arg)-1;
					break;
				}
				case 'p': {
					char buf[17];
					uint_to_str(va_arg(arg_list, u64), buf, 16);
					memcpy(dest+dest_c, buf, strlen(buf));
					dest_c += strlen(buf)-1;
					break;
				}
				default: {
					if (fmt[fmt_c] <= '9' && fmt[fmt_c] >= '0') {
						uint8_t first = (uint8_t)(fmt[++fmt_c]) - (uint8_t)'0';
						uint8_t second = (uint8_t)(fmt[++fmt_c]) - (uint8_t)'0';
						uint8_t final = second + (first * 10);
						switch (fmt[fmt_c]) {
							case 'x': {
								char buffer[final + 1];
								memset(buffer, 0, final + 1);
								u64 arg = va_arg(arg_list, uint64_t);
								uint_to_str(arg, buffer, 16);
								memcpy(dest+dest_c, buffer, final);
								dest_c += strlen(buffer)-1;
								break;
							}
							case 'X': {
								char buffer[final + 1];
								memset(buffer, 0, final + 1);
								u64 arg = va_arg(arg_list, uint64_t);
								uint_to_str(arg, buffer, 16);
								memcpy(dest+dest_c, buffer, final);
								dest_c += strlen(buffer)-1;
								break;
							}
							case 's': {
								memcpy(dest+dest_c, va_arg(arg_list, char*), final);
								dest_c += final;
								break;
							}
							case 'b': {
								dest[dest_c] = va_arg(arg_list, uint64_t) == 1 ? '1' : '0';
								dest_c++;
								break;
							};
							default:
								break;
						}
					}
				}
				break;
			}
		} else {
			dest[dest_c] = fmt[fmt_c];
		}

		if(fmt_c == strlen(fmt)) {
			dest[dest_c+1] = '\0';
			break;
		}
	}

	va_end(arg_list);
}
