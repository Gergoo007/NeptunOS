#pragma once

#include "stdint.h"
#include "stdarg.h"

void debug_putc(const char c);
void debug_puts(const char* str);
void printk(char* fmt, ...);
