#pragma once

#include <lib/sprintf.h>

void putc(char c);
void puts(const char *restrict s);
_attr_printf void sprintk(const char* fmt, ...);
