#pragma once

#include <arch/arch.hh>

#define COM1 0x3f8

void sinit();
void sputc(const char c);
char sgetc();
