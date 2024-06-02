#pragma once

#include <lib/types.h>
#include <lib/types.h>
#include <lib/string.h>

#include <lib/attrs.h>

char* sprintf(char* out, const char* fmt, ...);
char* vsprintf(char* out, const char* fmt, va_list args);
