#pragma once

#include <lib/types.h>
#include <serial/serial.h>

void strdmp(char* s, u16 len);
u8 strlen(const char* str);
u8 wstrlen(const wchar* str);
void utf16_to_ascii(wchar* in, char* out);
void utf16_to_asciin(wchar* in, char* out, u32 n);
void strcpy(const char* src, char* dest);
u8 strncmp(const char* s1, const char* s2, u32 chars);
void strcat(char* dest, char* src);
void int_to_str(i64 i, char* str);
void uint_to_str(u64 i, char* str);
void hex_to_str(u64 i, char* str);
void hexn_to_str(u64 i, char* str, u8 num);
