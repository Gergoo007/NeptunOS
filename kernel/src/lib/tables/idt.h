#pragma once

#include <stdint.h>

#define IDT_TA_INTERRUPT_GATE 	0b10001110
#define IDT_TA_CALL_GATE 		0b10001100
#define IDT_TA_TRAP_GATE 		0b10001111

typedef struct idt_desc_entry {
	uint16_t offset0;
	uint16_t selector;
	uint8_t ist; // interrupt stack table
	uint8_t attrib;
	uint16_t offset1;
	uint32_t offset2;
	uint32_t none;
} idt_desc_entry;

typedef struct idtr {
	uint16_t limit;
	uint16_t offset;
} __attribute__((packed)) idtr;

void set_offset(uint64_t offset, idt_desc_entry* entry);
uint64_t get_offset(idt_desc_entry* entry);
