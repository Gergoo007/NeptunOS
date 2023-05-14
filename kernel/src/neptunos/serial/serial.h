#pragma once

#include <neptunos/libk/stdall.h>

extern void serial_init(void);

extern void serial_write(uint16_t port, char* msg);
extern void serial_write_c(uint16_t port, char msg);
extern void serial_write_d(uint16_t port, int msg);

extern void _debug_puts(const char *str);
extern void _debug_putc(const char c);
extern void _serprintk(char* fmt, ...);

#ifdef SERIAL_DEBUG
#define debug_puts _debug_puts
#define debug_putc _debug_putc
#define serprintk _serprintk
#else
#define debug_puts(c, ...) asm("nop")
#define debug_putc(c, ...) asm("nop")
#define serprintk(c, ...) asm("nop")
#endif
