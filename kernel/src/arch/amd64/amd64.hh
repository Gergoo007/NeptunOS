#pragma once

#include <arch/arch.hh>

#define COM1 0x3f8

void sinit();
void sputc(const char c);
char sgetc();
void arch_sleep(u64 ms);
u64 arch_get_time();
bool arch_elapsed(u64 timestamp, u64 ms);
