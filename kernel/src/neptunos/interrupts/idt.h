#pragma once

#include <neptunos/libk/stdint.h>
#include <neptunos/config/attributes.h>
#include <neptunos/globals.h>
#include "interrupts.h"
#include <neptunos/memory/paging.h>

typedef struct idt_desc {
	u16 offs0;
	u16 ss;
	u8 ist : 3;
	u8 : 5;
	u8 gate_type : 4;
	u8 : 1;
	u8 dpl : 2;
	u8 p : 1;
	u16 offs1;
	u32 offs2;
	u32 : 32;
} _attr_packed idt_desc_t;

typedef struct idtr {
	u16 limit; // size - 1
	u64 addr;
} _attr_packed idtr_t;

extern idt_desc_t* idt;

void idt_init(void);
u8 idt_add_isr(void (*handler)(struct interrupt_frame*));
void idt_add_entry(void (*handler)(struct interrupt_frame*), u8 vector);
