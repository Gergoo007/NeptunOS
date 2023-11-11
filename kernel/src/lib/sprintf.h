#pragma once

#include <lib/int.h>
#include <lib/stdarg.h>
#include <lib/string.h>

#include <lib/attrs.h>

__attribute__((format(printf, 2, 3))) void sprintf(char* out, const char* fmt, ...);
void vsprintf(char* out, const char* fmt, va_list args);
