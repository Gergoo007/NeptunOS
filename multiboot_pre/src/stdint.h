#pragma once

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef long long i64;
typedef int i32;
typedef short i16;
typedef char i8;

typedef u64 size_t;

typedef u64 uintptr_t;
typedef i64 intptr_t;

char* uint_to_str(u64 value, char* result, int base);

char* int64_to_str(i64 value, char* result, int base);

void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t len);
int memcmp(const void *vl, const void *vr, size_t n);
