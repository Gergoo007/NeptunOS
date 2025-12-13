#pragma once

#include <types.hh>
#include <arch/amd64/io.hh>

static bool cpuid_is_emu() {
	u32 eax = 0x40000000, ebx, ecx, edx;
	asm volatile ("cpuid" : "=c"(ecx), "=d"(edx), "=b"(ebx) : "a"(eax));

	if (ebx == 'KMVK')
		return true;
	if (ebx == 'TGCT')
		return true;

	return false;
}
