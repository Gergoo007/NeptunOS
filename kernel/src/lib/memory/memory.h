#pragma once

#include <stdint.h>

void map_memory();

void init_bitmap(uint8_t* bitmap, uint64_t _bitmap_size);

extern uint64_t total_mem;
extern uint64_t free_mem;
extern uint64_t used_mem;
extern uint64_t num_pages;

extern uint8_t* bitmap;
