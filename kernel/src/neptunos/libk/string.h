#pragma once

#include <neptunos/libk/stdint.h>
#include <neptunos/libk/stddef.h>

int str_to_int(const char* str, int* num);

char* int_to_str(int value, char* result, int base);

char* int64_to_str(int64_t value, char* result, int base);

char* uint_to_str(uint64_t value, char* result, int base);

void* memcpy(void* dest, const void* src, size_t n);

void* memset(void* dest, const int src, size_t n);

uint8_t strncmp(char* str1, char* str2, uint16_t len);
