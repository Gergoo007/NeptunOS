#pragma once

#include <lib/int.h>

u8 strlen(const char* str);
void strcpy(const char* src, char* dest);
u8 strncmp(const char* s1, const char* s2, u32 chars);
void int_to_str(i64 i, char* str);
void uint_to_str(u64 i, char* str);
void hex_to_str(u64 i, char* str);
void ptr64_to_str(u64 i, char* str);
