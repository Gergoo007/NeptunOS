#include <lib/string.h>

// Szöveg hossza a null terminátor nélkül
u8 strlen(const char* str) {
	u8 len = 1;
	while (*(str++))
		len++;

	return len;
}

void strcpy(const char* src, char* dest) {
	while (*src != '\0') {
		*dest = *src;
		src++;
		dest++;
	}
	*dest = '\0';
}

u8 strncmp(const char* s1, const char* s2, u32 chars) {
	while (chars--) {
		if (*s1 != *s2)
			return 1;
	}
	return 0;
}

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

void hexn_to_str(u64 i, char* str, u8 num) {
	const char* numbers = "0123456789abcdef";
	u8 len = 1;

	// Számokat át konvertáljuk karakterekké
	for (u8 j = 0; j < num; j++, i /= 16) {
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
