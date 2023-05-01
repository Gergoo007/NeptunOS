#pragma once

#include "mmap.h"

typedef uint64_t size_t;

typedef struct bitmap_t {
	void* address;
	uint64_t size;
} bitmap_t;

typedef struct mem_block_hdr_t {
	uint16_t pid;
	uint64_t pages;
} mem_block_hdr_t;

void init_pmm(multiboot_tag_efi_mmap_t* mmap_tag);
uint8_t bm_get(uint64_t page);
void bm_set(uint64_t page, uint8_t val);

void free_page(void* address, uint64_t count);
void lock_page(void* address, uint64_t count);
void reserve_page(void* address, uint64_t count);
void unreserve_page(void* address, uint64_t count);
void* request_page();

void* malloc(uint64_t size_in_bytes);
// Use it carefully as overflow is not accounted for
void* calloc(size_t members, size_t member_size);
void free(void* addr);
