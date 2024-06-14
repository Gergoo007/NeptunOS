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
} gdt_entry;

typedef struct gdt {
	gdt_entry knull; // 0x00
	gdt_entry kcode; // 0x08
	gdt_entry kdata; // 0x10
	gdt_entry unull;
	gdt_entry ucode;
	gdt_entry udata;
} gdt;

typedef struct gdtr {
	u16 limit;
	u64 base;
} gdtr;

void gdt_init(void);
void gdt_load(gdtr* gdtr);
