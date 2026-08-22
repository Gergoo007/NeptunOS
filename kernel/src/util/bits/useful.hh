// random bs I don't feel like including every time

#pragma once

void arch_halt();
void arch_cli();
[[noreturn]] static inline void pause() { while (1) { arch_cli(); arch_halt(); } }

#define pstruct struct attr_packed
#define punion union attr_packed

#include <util/bits/arithmetic.hh>

extern u8 _binary_src_font_psf_start;
extern u8 _binary_src_font_psf_end;

#define FONTFILE_START &_binary_src_font_psf_start
#define FONTFILE_END &_binary_src_font_psf_end

constexpr u64 P4K = 0x1000;
constexpr u64 P2M = 0x200000;

// thank you osdev.org
static inline int oct2bin(u8* str, int size) {
    int n = 0;
    while (size-- > 0) {
        n *= 8;
        n += *(str++) - '0';
    }
    return n;
}

void sched_setrunning(bool otoole);
