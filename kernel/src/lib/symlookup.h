#pragma once

#include <lib/types.h>
#include <lib/kinfo.h>

#include <graphics/console.h>

#pragma pack(1)

typedef struct symbol {
	u32 st_name;
	u8 st_info;
	u8 st_other;
	u16 st_shndx;
	u64 st_value;
	u64 st_size;
} symbol;

typedef struct stackframe {
	struct stackframe* rbp;
	u64 rip;
} stackframe;

char* sym_at_addr(u64 addr);
u64 sym_nearest_sym(u64 addr);
void stacktrace(stackframe* rbp, char* buf);
