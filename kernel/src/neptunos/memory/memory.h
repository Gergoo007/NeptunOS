#pragma once

#include <neptunos/libk/stdint.h>
#include <neptunos/config/attributes.h>

typedef struct bitmap_t {
	void* address;
	uint64_t size;
} bitmap_t;

typedef struct mem_block_hdr_t {
	uint16_t pid;
	uint64_t pages;
} mem_block_hdr_t;

void map_memory();

void init_bitmap(bitmap_t* bm);

void free_page(void* address, uint64_t count);

void lock_page(void* address, uint64_t count);

void reserve_page(void* address, uint64_t count);

void unreserve_page(void* address, uint64_t count);

void* request_page();

uint8_t bm_get(uint64_t page);

void bm_set(uint64_t page, uint8_t val);

void* malloc(uint64_t size_in_bytes);
void free(void* addr);

extern uint64_t total_mem;
extern uint64_t free_mem;
extern uint64_t reserved_mem;
extern uint64_t used_mem;
extern uint64_t num_pages;

extern void* biggest_conv_mem;
extern uint64_t biggest_conv_mem_size;
