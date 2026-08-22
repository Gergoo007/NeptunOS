#pragma once

#define va_start(v, l)	__builtin_va_start(v,l)
#define va_end(v)		__builtin_va_end(v)
#define va_arg(v,l)		__builtin_va_arg(v,l)
#define va_list __builtin_va_list

#define offsetof(s, m) __builtin_offsetof(s, m)

#define attr_noret __attribute__((noreturn))
#define attr_packed __attribute__((packed))
#define attr_interrupt __attribute__((interrupt))
