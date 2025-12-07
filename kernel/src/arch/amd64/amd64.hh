#pragma once

#include <arch/arch.hh>

#define COM1 0x3f8

void sinit();
void sputc(const char c);
char sgetc();
void arch_sleep(u64 ms);
void arch_start_timer();
u64 arch_ms_passed();
bool arch_elapsed(u64 ms);
