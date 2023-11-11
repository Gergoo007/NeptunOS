#pragma once

#include <lib/int.h>

#pragma pack(1)

typedef struct kernel_info {
	u64 kernel_addr;
	u64 preload_addr;
	u64 mb_hdr_addr;
} kernel_info_t;

extern kernel_info_t* info;
