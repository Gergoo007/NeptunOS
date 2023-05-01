#pragma once

typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef long long int64_t;
typedef int int32_t;
typedef short int16_t;
typedef char int8_t;

typedef uint64_t size_t;

typedef uint64_t uintptr_t;
typedef int64_t intptr_t;

char* uint_to_str(uint64_t value, char* result, int base);

char* int64_to_str(int64_t value, char* result, int base);

void* memcpy(void* dest, const void* src, size_t n);

void* memset(void* s, int c, size_t len);
