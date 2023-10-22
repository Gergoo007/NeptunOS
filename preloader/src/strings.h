#pragma once

#include <int.h>

u8 strlen(const char* str);
void strcpy(const char* src, char* dest);
u8 strncmp(const char* s1, const char* s2, u32 chars);
void memcpy(const u8* src, u8* dest, u64 i);
void memset(u8* dest, const u8 src, u64 i);
