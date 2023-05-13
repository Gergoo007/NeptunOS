#include "stdint.h"

char* uint_to_str(u64 value, char* result, int base) {
	// check that the base if valid
	if (base < 2 || base > 36) { *result = '\0'; return result; }

	char* ptr = result, *ptr1 = result, tmp_char;
	u64 tmp_value;

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

char* int64_to_str(i64 value, char* result, int base) {
	// check that the base if valid
	if (base < 2 || base > 36) { *result = '\0'; return result; }

	char* ptr = result, *ptr1 = result, tmp_char;
	i64 tmp_value;

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

void* memcpy(void* p, const void* src, size_t n) {
	for (size_t i = 0; i < n; i++) {
		*((u8*)p++) = *((u8*)src++);
	}
	return p;
}

void* memset(void* p, int c, size_t n) {
	for (; n; n--, p++)
		*(u32*)p = c;

	return p;
}

int memcmp(const void* one, const void* two, size_t n) {
	for (; n; n--) {
		if (((u8*)one)[n] != ((u8*)two)[n])
			return 0;
	}
	return 1;
}
