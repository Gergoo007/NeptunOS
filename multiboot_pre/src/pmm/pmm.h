#pragma once

#include "mmap.h"

typedef u64 size_t;

typedef struct bitmap_t {
	void* address;
	u64 size;
} bitmap_t;

typedef struct mem_block_hdr_t {
	u16 pid;
	u64 pages;
} mem_block_hdr_t;

void init_pmm(multiboot_tag_efi_mmap_t* mmap_tag);
u8 bm_get(u64 page);
void bm_set(u64 page, u8 val);

void free_page(void* address, u64 count);
void lock_page(void* address, u64 count);
void reserve_page(void* address, u64 count);
void unreserve_page(void* address, u64 count);
void* request_page();

void* malloc(u64 size_in_bytes);
// Use it carefully as overflow is not accounted for
void* calloc(size_t members, size_t member_size);
void free(void* addr);
