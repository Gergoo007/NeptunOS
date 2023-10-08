// typedef __builtin_va_list __gnuc_va_list;
// typedef __builtin_va_list va_list;

// #if (__STDC_VERSION__ >= 202000L)
// 	#define va_start(ap, ...) __builtin_va_start(ap, 0)
// #else
// 	#define va_start(ap, param) __builtin_va_start(ap, param)
// #endif

// #define va_end(ap)			__builtin_va_end(ap)
// #define va_arg(ap, type)	__builtin_va_arg(ap, type)

// #define __va_copy(d,s) __builtin_va_copy(d,s)

// #if (__STDC_VERSION__ >= 199901L) || !defined(__STRICT_ANSI__)
// 	#define va_copy(dest, src)  __builtin_va_copy(dest, src)
// #endif

#pragma once

typedef __builtin_va_list va_list;

typedef __builtin_va_list 	__gnuc_va_list;

#define va_start(ap, arg) 	__builtin_va_start(ap, arg)
#define va_end(ap)			__builtin_va_end(ap)
#define va_arg(ap, type)	__builtin_va_arg(ap, type)

#define __va_copy(dest,source)		__builtin_va_copy(dest,source)
#define va_copy(dest, source)		__builtin_va_copy(dest, source)
