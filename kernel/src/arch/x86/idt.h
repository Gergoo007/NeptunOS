#pragma once

#include <lib/types.h>

#include <memory/heap/vmm.h>

#include <arch/x86/interrupts.h>

#pragma pack(1)

enum {
	IDT_TYPE_INT = 0xe,
	IDT_TYPE_TRAP = 0xf,
};

typedef struct idtr {
	u16 limit;
	u64 base;
} idtr_t;

typedef struct idt_entry {
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
} idt_entry_t;

u8 idt_reserve_vector(void* isr);
void idt_attach_isr(u8 vector, void* isr);
void idt_create_entry(idt_entry_t* entry, u64 addr, u8 dpl, u8 type);
void idt_init(void);
void idt_load(idtr_t* idtr);
