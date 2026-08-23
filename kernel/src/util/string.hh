#pragma once

#include <types.hh>

// Szöveg hossza a null terminátor nélkül
constexpr u8 strlen(const char* str) {
	u8 len = 0;
	while (*(str++))
		len++;

	return len;
}

// Szöveg hossza de a szöveg UTF-16/UCS-2
constexpr u8 wstrlen(const wchar_t* str) {
	u8 len = 0;
	while (*(str++))
		len++;

	return len;
}

u32 ucs2_to_ascii(wchar* in, char* out);
void ucs2_to_asciin(wchar* in, char* out, u32 n);
u32 strcpy(const char* src, char* dest);
u8 strncmp(const char* s1, const char* s2, u32 chars);
u8 strcmp(const char* s1, const char* s2);
void strcat(char* dest, char* src);

[[nodiscard]]
constexpr i64 str_to_int(const char* str, u32 base = 10) {
	i64 ret = 0;
	bool neg = false;

	if (*str == '-') {
		neg = true;
		str++;
	}

	if (*str == '0' && *(str + 1) == 'x') {
		base = 16;
		str += 2;
	}

	while ((base <= 10 && ISDIGIT(*str)) || (base > 10 && (ISLETTER(*str) || ISDIGIT(*str)))) {
		ret *= base;
		if (ISUPPERCASE(*str)) {
			ret += *(str++) - 'A' + 10;
		} else if (ISLOWERCASE(*str)) {
			ret += *(str++) - 'a' + 10;
		} else {
			ret += *(str++) - '0';
		}
	}

	if (neg) ret *= -1;
	return ret;
}

[[nodiscard]]
constexpr u64 str_to_uint(const char* str, u32 base = 10) {
	u64 ret = 0;

	if (*str == '-') {
		fatal("str_to_uint on a negative");
	}

	if (*str == '0') {
		if (*(str + 1) == 'x') {
			base = 16;
			str += 2;
		} else if (*(str + 1) == '0') {
			base = 8;
			str += 2;
		} else if (*(str + 1) == 'b') {
			base = 2;
			str += 2;
		}
	}

	while ((base <= 10 && ISDIGIT(*str)) || (base > 10 && (ISLETTER(*str) || ISDIGIT(*str)))) {
		ret *= base;
		if (ISUPPERCASE(*str)) {
			ret += *(str++) - 'A' + 10;
		} else if (ISLOWERCASE(*str)) {
			ret += *(str++) - 'a' + 10;
		} else {
			ret += *(str++) - '0';
		}
	}

	return ret;
}
