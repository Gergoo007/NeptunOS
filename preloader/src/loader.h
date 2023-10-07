#pragma once

#pragma pack(1)

#include <int.h>

typedef struct kernel_info {
	u64 kernel_addr;
	u64 preload_addr;
	u64 mb_hdr_addr;
} kernel_info_t;
