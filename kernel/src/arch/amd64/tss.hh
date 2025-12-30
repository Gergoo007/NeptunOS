#pragma once

#include <types.hh>

pstruct tss_t {
	u32 : 32;
	u64 rsp0;
	u64 rsp1;
	u64 rsp2;
	u32 : 32;
	u32 : 32;
	u64 ist1;
	u64 ist2;
	u64 ist3;
	u64 ist4;
	u64 ist5;
	u64 ist6;
	u64 ist7;
	u64 : 64;
	u16 : 16;
	u16 io_bm_offset;

	pstruct { // todo

	} io_bm;
};

struct gdt_entry_t;
void arch_tss_init(gdt_entry_t* gdt);
