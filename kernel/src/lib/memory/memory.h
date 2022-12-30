#pragma once

#include <stdint.h>

typedef struct bitmap {
	void* address;
	uint64_t size;
} bitmap;

void map_memory();

void init_bitmap(bitmap* bm);

void free_page(void* address, uint64_t count);

void lock_page(void* address, uint64_t count);

void reserve_page(void* address, uint64_t count);

void unreserve_page(void* address, uint64_t count);

void* request_page();

uint8_t bm_get(uint64_t page);

void bm_set(uint64_t page, uint8_t val);

void* malloc(uint64_t size_in_bytes);

extern uint64_t total_mem;
extern uint64_t free_mem;
extern uint64_t reserved_mem;
extern uint64_t used_mem;
extern uint64_t num_pages;

extern void* biggest_conv_mem;
extern uint64_t biggest_conv_mem_size;
