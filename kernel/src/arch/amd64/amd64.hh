#pragma once

#include <types.hh>

#define COM1 0x3f8

struct cpu_state_amd64_t {
	u64 cr3 = 0;
	u64 r15 = 0;
	u64 r14 = 0;
	u64 r13 = 0;
	u64 r12 = 0;
	u64 r11 = 0;
	u64 r10 = 0;
	u64 r9 = 0;
	u64 r8 = 0;
	u64 cr2 = 0;
	u64 rbp = 0;
	u64 rsi = 0;
	u64 rdi = 0;
	u64 rdx = 0;
	u64 rcx = 0;
	u64 rbx = 0;
	u64 rax = 0;
	u64 exc = 0;
	u64 rflexc = 0;
	u64 err = 0;
	u64 rip = 0;
	u64 cs = 0;
	u64 rfl = 0;
	u64 rsp = 0;
	u64 ss = 0;
};

pstruct thread_info {
	u32 task_id;
};

struct Amd64Msrs {
	static constexpr u32 EFER = 0xC0000080;
	static constexpr u32 FSBase = 0xC0000100;
	static constexpr u32 GSBase = 0xC0000101;
	static constexpr u32 KernelGSBase = 0xC0000102;
};

extern "C" void sse_init();
void sinit();
void sputc(const char c);
char sgetc();
void arch_sleep(u64 ms, bool skippable = false);
void arch_start_timer();
u64 arch_ms_passed();
bool arch_elapsed(u64 ms);

static inline void amd64_set_msr(u64 msr, u64 val) { asm volatile ("wrmsr" :: "c"(msr), "a"((u32)val), "d"((u32)(val >> 32))); }

static inline u64 amd64_get_msr(u64 msr) {
	u32 hi, lo;
	asm volatile ("wrmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
	return lo | (((u64)hi) << 32);
}
