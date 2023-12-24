#pragma once

#include <lib/sprintf.h>

#include <memory/heap/vmm.h>

void putc(char c);
void puts(const char* s);
void puts_translate(const char* s);
void sprintk(const char* fmt, ...);
