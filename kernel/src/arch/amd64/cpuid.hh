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

static bool cpuid_is_intel() {
	u32 eax = 0x00, ebx, ecx, edx;
	asm volatile ("cpuid" : "=c"(ecx), "=d"(edx), "=b"(ebx) : "a"(eax));
	return ebx == 'uneG' && edx == 'Ieni' && ecx == 'letn';
}

static bool cpuid_is_amd() {
	u32 eax = 0x00, ebx, ecx, edx;
	asm volatile ("cpuid" : "=c"(ecx), "=d"(edx), "=b"(ebx) : "a"(eax));
	return ebx == 'htuA' && edx == 'itne' && ecx == 'DMAc';
}

static u32 cpuid_max_leaf() {
	u32 eax = 0x00, ebx, ecx, edx;
	asm volatile ("cpuid" : "=a"(eax), "=c"(ecx), "=d"(edx), "=b"(ebx) : "a"(eax));
	return eax;
}

static bool cpuid_pcid_supported() {
	u32 eax = 0x01, ebx, ecx, edx;
	asm volatile ("cpuid" : "=c"(ecx), "=d"(edx), "=b"(ebx) : "a"(eax));
	return ecx & (1 << 17);
}

static u32 cpuid_x2apic_id() {
	u32 eax = 0x1f, ebx, ecx, edx;
	if (cpuid_is_amd()) eax = 0x0b;
	asm volatile ("cpuid" : "=c"(ecx), "=d"(edx), "=b"(ebx) : "a"(eax));
	return edx;
}

static bool cpuid_x2apic_supported() {
	u32 eax = 0x01, ebx, ecx, edx;
	asm volatile ("cpuid" : "=c"(ecx), "=d"(edx), "=b"(ebx) : "a"(eax));
	return ecx & (1 << 21);
}

static u32 cpuid_xapic_id() {
	u32 eax = 0x01, ebx, ecx, edx;
	asm volatile ("cpuid" : "=c"(ecx), "=d"(edx), "=b"(ebx) : "a"(eax));
	return ebx >> 24;
}

