#pragma once

#include <neptunos/libk/stdint.h>
#include <neptunos/config/attributes.h>

typedef struct gdt_descriptor {
	uint16_t size;
	uint64_t offset;
} _attr_packed gdt_descriptor;

typedef struct gdt_entry {
	uint16_t limit0;
	uint16_t base0;
	uint8_t base1;
	uint8_t access_byte;
	uint8_t limit1_flags;
	uint8_t base2;
} _attr_packed gdt_entry_t;

typedef struct gdt {
	gdt_entry_t null;
	gdt_entry_t kernel_code;
	gdt_entry_t kernel_data;
	gdt_entry_t user_null;
	gdt_entry_t user_code;
	gdt_entry_t user_data;
} _attr_packed __attribute__((aligned(0x1000))) gdt;

extern struct gdt gdt_obj;

void load_gdt(struct gdt_descriptor* gdt_desc);
