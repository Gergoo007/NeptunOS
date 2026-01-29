#pragma once

#include <types.hh>
#include <util/async.hh>
#include <arch/amd64/amd64.hh>

#define DECL_ISR(n) extern "C" attr_interrupt void exc##n(cpu_state_amd64_t* i)

pstruct idt_entry_t {
	u16 base0;
	u16 ss;
	u8 ist : 3;
	u8 : 5;
	u8 gate_type : 4;
	u8 : 1;
	u8 dpl : 2;
	u8 present : 1;
	u16 base1;
	u32 base2;
	u32 : 32;
};

pstruct idtr_t {
	u16 size; // size-1
	idt_entry_t* address;
};

DECL_ISR(0);
DECL_ISR(1);
DECL_ISR(2);
DECL_ISR(3);
DECL_ISR(4);
DECL_ISR(5);
DECL_ISR(6);
DECL_ISR(7);
DECL_ISR(8);
DECL_ISR(9);
DECL_ISR(10);
DECL_ISR(11);
DECL_ISR(12);
DECL_ISR(13);
DECL_ISR(14);
DECL_ISR(15);
DECL_ISR(16);
DECL_ISR(17);
DECL_ISR(18);
DECL_ISR(19);

DECL_ISR(64);
DECL_ISR(65);
DECL_ISR(66);

extern atomic<u64> tmr_counter;

void arch_idt_init();
