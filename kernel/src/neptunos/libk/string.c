#include <neptunos/libk/string.h>

#include <neptunos/libk/stdint.h>
#include <neptunos/libk/stdarg.h>
#include <neptunos/libk/limits.h>
#include <neptunos/libk/stdint.h>

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

void* memset(void* dest, const int src, size_t n) {
	for (; n; n--, dest++)
		*((uint32_t*)dest) = src;

	return dest;
}

uint8_t strncmp(char* str1, char* str2, uint16_t len) {
	uint8_t result = 0;
	while((len--) > 0) {
		result |= (str1[len] ^ str2[len]);
	}
	return !result;
}


// void sprintf(char* string, char* fmt, ...) {
// 	va_list arg_list;
// 	va_start(arg_list, fmt);
// 	int16_t length = 0;
// 	uint16_t character = 0;

// 	while(*fmt != '\0') {
// 		if (*fmt == '%') {
// 			fmt++;
// 			switch (*fmt) {
// 				case 'c':
// 					string[character] = ((char)va_arg(arg_list, uint32_t));
// 					break;
// 				case 'd': {
// 					char res[256] = "";
// 					int64_to_str(va_arg(arg_list, int64_t), res, 10);
// 					break;
// 				}
// 				case 'u':
// 					fmt++;
// 					switch(*fmt) {
// 						case 'd': {
// 							char res[256] = "";
// 							render_string(uint_to_str(va_arg(arg_list, uint64_t), res, 10));
// 							break;
// 						}
// 					}
// 					break;
// 				case 's':
// 					printk(va_arg(arg_list, char*));
// 					break;
// 				case 'p': {
// 					char testbuf[17];
// 					fmt_x(testbuf, (uint64_t)va_arg(arg_list, void*), 15, 1);
// 					render_string(testbuf);
// 					break;
// 				}
// 				default: {
// 						if (*fmt <= '9' && *fmt >= '0') {
// 							uint8_t first = (uint8_t)(*fmt++) - (uint8_t)'0';
// 							uint8_t second = (uint8_t)*(fmt++) - (uint8_t)'0';
// 							uint8_t final = second + (first * 10);

// 							switch (*fmt) {
// 								case 'x': {
// 									char buffer[final + 1];
// 									memset(buffer, 0, final + 1);
// 									fmt_x(buffer, (uint64_t)va_arg(arg_list, uint64_t), final, 0);
// 									render_string(buffer);
// 									break;
// 								}
// 								case 'X': {
// 									char buffer[final + 1];
// 									memset(buffer, 0, final + 1);
// 									fmt_x(buffer, (uint64_t)va_arg(arg_list, uint64_t), final, 1);
// 									render_string(buffer);
// 									break;
// 								}
// 								case 's': {
// 									char buffer[final + 1];
// 									memcpy(buffer, (void*)va_arg(arg_list, char*), final);
// 									buffer[final] = '\0';
// 									render_string(buffer);
// 									break;
// 								}
// 								case 'b': {
// 									uint64_t num = va_arg(arg_list, uint64_t);
// 									while (final-- > 0) {
// 										if (num & (1 << final))
// 											render_char('1');
// 										else
// 											render_char('0');
// 									}
// 									break;
// 								};
// 								default:
// 									render_string("Invalid format: ");
// 									render_char(*fmt);
// 									render_string("!\n");
// 									break;
// 							}
// 						}
// 					}
// 					break;
// 			}
// 		} else {
// 			render_char(*fmt);
// 			length++;
// 		}

// 		fmt++;

// 		if (cursor_y >= info->g_info->info->height) {
// 			cursor_y = 0;
// 		}
// 	}
// }
