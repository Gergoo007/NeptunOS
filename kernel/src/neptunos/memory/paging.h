#pragma once

#include <neptunos/libk/stdint.h>

typedef struct page_map_entry {
	uint8_t present 		: 1;
    uint8_t read_write 		: 1;
    uint8_t user_super 		: 1;
    uint8_t write_through 	: 1;
    uint8_t cache_disabled 	: 1;
    uint8_t accessed 		: 1;
    uint8_t ignore0 		: 1;
    uint8_t larger_pages 	: 1;
	uint8_t ignore1 		: 1;
    uint8_t available 		: 3;
    uint64_t address 		: 48;
	uint8_t pad 			: 4; // To make it exactly 64 bits
} __attribute__((packed)) page_map_entry;

typedef struct page_map_level {
	page_map_entry entries[512];
} __attribute__((packed)) page_map_level;

extern page_map_level* pml4;

void indexer(void* address, uint16_t* pt_i, uint16_t* pd_i, uint16_t* pdp_i, uint16_t* pml4_i);
void map_address(void* virtual_address, void* physical_address);
