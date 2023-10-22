#pragma once

#pragma pack(1)

#include <int.h>
#include <elf.h>
#include <strings.h>
#include <serial.h>
#include <paging.h>
#include <memory.h>

extern u64 _binary_out_kernel_start;
extern u64 _binary_out_kernel_end;

typedef struct kernel_info {
	u64 kernel_addr;
	u64 preload_addr;
	u64 mb_hdr_addr;
} kernel_info_t;

u64 load_kernel(void);
