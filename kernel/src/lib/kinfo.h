#pragma once

#include <lib/types.h>

#pragma pack(1)

typedef struct kernel_info {
	u64 kernel_addr;
	u64 preload_addr;
	u64 mb_hdr_addr;
	u64 strtab1;
	u64 strtab2;
	u64 symtab;
	u16 symtab_entries;
} kernel_info_t;

extern kernel_info_t* info;
u8 kmain(kernel_info_t* _info);
