#pragma once

#include <int.h>
#include <stdarg.h>

void putc(char c);
void puts(const char* restrict s);
void int_to_str(i64 i, char* str);
void uint_to_str(u64 i, char* str);
void hex_to_str(u64 i, char* str);
void ptr64_to_str(u64 i, char* str);
void printf(const char* fmt, ...);
