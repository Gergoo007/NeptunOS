#pragma once

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef __uint128_t u128;

typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;
typedef __int128_t i128;

typedef u8 uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;
typedef u64 uint64_t;

typedef i8 int8_t;
typedef i16 int16_t;
typedef i32 int32_t;
typedef i64 int64_t;

typedef u16 wchar;

typedef u64 uintptr_t;

typedef __SIZE_TYPE__ size_t;

#define UINT64_C(v) v##ULL
