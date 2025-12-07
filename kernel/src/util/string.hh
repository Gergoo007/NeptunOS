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
constexpr u8 wstrlen(const wchar* str) {
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
void int_to_str(i64 i, char* str);
void uint_to_str(u64 i, char* str);
void uintn_to_str(u64 i, char* str, u8 num);
void hex_to_str(u64 i, char* str);
void hexn_to_str(u64 i, char* str, u8 num);
u64 str_to_uint(const char* s, u64* numlen = nullptr);
