#pragma once

#include <lib/int.h>
#include <lib/stdarg.h>
#include <lib/string.h>

#include <lib/attrs.h>

void sprintf(char* out, const char* fmt, ...);
void vsprintf(char* out, const char* fmt, va_list args);
