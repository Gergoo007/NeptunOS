#pragma once

#include <lib/macros.h>

#define NULL (void*)0

typedef unsigned long long 	u64;
typedef unsigned int 		u32;
typedef unsigned short 		u16;
typedef unsigned char 		u8;

typedef long long 			i64;
typedef int 				i32;
typedef short 				i16;
typedef char 				i8;

typedef short				wchar;

typedef __builtin_va_list va_list;

typedef __builtin_va_list 	__gnuc_va_list;

#define va_start(ap, arg) 	__builtin_va_start(ap, arg)
#define va_end(ap)			__builtin_va_end(ap)
#define va_arg(ap, type)	__builtin_va_arg(ap, type)

#define __va_copy(dest,source)		__builtin_va_copy(dest,source)
#define va_copy(dest, source)		__builtin_va_copy(dest, source)

typedef struct regs {
	u64 r15;
	u64 r14;
	u64 r13;
	u64 r12;
	u64 r11;
	u64 r10;
	u64 r9;
	u64 r8;
	u64 cr2;
	u64 rbp;
	u64 rsi;
	u64 rdi;
	u64 rdx;
	u64 rcx;
	u64 rbx;
	u64 rax;
	u64 exc;
	u64 err;
	u64 rip;
	u64 cs;
	u64 rfl;
	u64 rsp;
	u64 ss;
} regs;
