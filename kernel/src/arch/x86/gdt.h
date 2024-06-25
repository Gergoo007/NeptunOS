#pragma once

#include <lib/types.h>
#include <lib/mem.h>
#include <memory/heap/vmm.h>
#include <arch/x86/tss.h>

#pragma pack(1)

// typedef struct gdt_entry {
// 	u64 : 40;

// 	u8 accessed : 1;		// 0
// 	u8 readable : 1;		// 1
// 	u8 conforming : 1;		// 0
// 	u8 executable : 1;		// 1
// 	u8 type : 1;			// 1
// 	u8 dpl : 2;				// 00
// 	u8 present : 1;			// 1

// 	u8 : 4;					// 0000
// 	u8 avl : 1;				// 0
// 	u8 longmode : 1;		// 1
// 	u8 def : 1;				// 0
// 	u8 granularity : 1;		// 1
// 	u8 : 8;
// } gdt_entry;

typedef struct _attr_packed gdt_entry {
	u16 limitl;
	u32 basel : 24;

	u8 accessed : 1;		// 0
	u8 readable : 1;		// 1
	u8 conforming : 1;		// 0
	u8 executable : 1;		// 1
	u8 type : 1;			// 1
	u8 dpl : 2;				// 00
	u8 present : 1;			// 1

	u8 limith : 4;					// 0000
	u8 avl : 1;				// 0
	u8 longmode : 1;		// 1
	u8 big : 1;				// 0
	u8 granularity : 1;		// 1
	u8 baseh;
} gdt_entry;

typedef struct _attr_packed gdt_entry_sys2 {
	u16 limit0;
	u16 base0;

	u8 base1;

	union {
		u8 access_byte;
		struct {
			u8 accessed : 1;
			u8 readable : 1;
			u8 conforming : 1;
			u8 executable : 1;
			u8 type : 1;
			u8 dpl : 2;
			u8 present : 1;
		};
	};

	u8 limit1 : 4;
	u8 avl : 1;
	u8 longmode : 1;
	u8 big : 1;
	u8 granularity : 1;
	u8 base2;
} gdt_entry_sys2;

typedef struct _attr_packed gdt_entry_sys1 {
	u32 base3;
	u32 : 32;
} gdt_entry_sys1;

typedef struct _attr_packed gdt {
	gdt_entry knull; // 0x00
	gdt_entry kcode; // 0x08
	gdt_entry kdata; // 0x10
	gdt_entry udata; // 0x18
	gdt_entry ucode; // 0x20
	gdt_entry_sys2 tss2; // 0x28
	gdt_entry_sys1 tss1; // 0x30
} gdt;

typedef struct gdtr {
	u16 limit;
	u64 base;
} gdtr;

void gdt_init(void);
void gdt_load(gdtr* gdtr);
void gdt_add_tss(tss* _tss);
