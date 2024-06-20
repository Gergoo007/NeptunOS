#pragma once

#pragma pack(1)

#include <int.h>
#include <elf.h>
#include <strings.h>
#include <serial.h>
#include <paging.h>
#include <memory.h>

typedef struct kernel_info {
	u64 kernel_addr;
	u64 preload_addr;
	u64 preload_end;
	u64 mb_hdr_addr;
	u64 strtab1;
	u64 strtab2;
	u64 symtab;
	u16 symtab_entries;
} kernel_info_t;

extern u64 _binary_out_kernel_start;
extern u64 _binary_out_kernel_end;
extern kernel_info_t kinfo;

u64 load_kernel(void);
