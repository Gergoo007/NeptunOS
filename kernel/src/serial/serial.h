#pragma once

#include <lib/sprintf.h>

#include <memory/heap/vmm.h>

void putc(char c);
void puts(const char* s);
void putc_translate(const char c);
void sprintk(const char* fmt, ...);
