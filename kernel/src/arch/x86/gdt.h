#pragma once

#include <lib/types.h>
#include <lib/mem.h>
#include <memory/heap/vmm.h>

#pragma pack(1)

typedef struct gdt_entry {
	u32 : 32;
	u8 : 8;

	u8 accessed : 1;		// 0
	u8 readable : 1;		// 1
	u8 conforming : 1;		// 0
	u8 executable : 1;		// 1
	u8 type : 1;			// 1
	u8 dpl : 2;				// 00
	u8 present : 1;			// 1

	u8 : 4;					// 0000
	u8 avl : 1;				// 0
	u8 longmode : 1;		// 1
	u8 def : 1;				// 0
	u8 granularity : 1;		// 1
	u8 : 8;
} gdt_entry_t;

typedef struct gdt {
	gdt_entry_t knull; // 0x00
	gdt_entry_t kcode; // 0x08
	gdt_entry_t kdata; // 0x10
	gdt_entry_t unull;
	gdt_entry_t ucode;
	gdt_entry_t udata;
} gdt_t;

typedef struct gdtr {
	u16 limit;
	u64 base;
} gdtr_t;

void gdt_init(void);
void gdt_load(gdtr_t* gdtr);
