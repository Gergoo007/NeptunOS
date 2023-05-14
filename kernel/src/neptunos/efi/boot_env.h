#pragma once

#include <neptunos/config/attributes.h>
#include <neptunos/libk/stdint.h>
#include <neptunos/libk/stddef.h>

#include <neptunos/efi/multiboot2.h>

typedef struct {
	multiboot_hdr_t* mb;
	struct {
		u64 total;
		u64 free;
		u64 used;
		u64 reserved;
		void* heap_base;
		u64 heap_size;
	} mem_stats;
	struct {
		void* kernel_addr;
	} kernel_stats;
} boot_info_t;
