#pragma once

#include <neptunos/libk/stdint.h>
#include <neptunos/config/attributes.h>

typedef enum {
	MAP_FLAG_PRESENT = 			0b000000000001,
	MAP_FLAG_RW = 				0b000000000010,
	MAP_FLAG_SUPER =			0b000000000100,
	MAP_FLAG_WRITE_THROUGH = 	0b000000001000,
	MAP_FLAG_CACHE_DISABLE = 	0b000000010000,
	MAP_FLAG_ACCESSED = 		0b000000100000,
	MAP_FLAG_AVAILABLE = 		0b000001000000,
	MAP_FLAG_4MB_PAGES = 		0b000010000000,
	MAP_FLAGS_DEFAULTS =		0 | MAP_FLAG_PRESENT | MAP_FLAG_RW,
	MAP_FLAGS_IO_DEFAULTS =		0 | MAP_FLAG_PRESENT | MAP_FLAG_RW | MAP_FLAG_CACHE_DISABLE,
} map_flags_t;

typedef struct page_map_entry {
	union {
		struct {
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
			uint8_t : 4; 	// To make it exactly 64 bits
		};
		struct {
			uint16_t flags : 12;
			uint64_t : 48; 	// Address, but it's already present in the other struct
			uint8_t : 4; 	// To make it exactly 64 bits
		};
	};
} _attr_packed page_map_entry;

typedef struct page_map_level {
	page_map_entry entries[512];
} _attr_packed page_map_level;

extern page_map_level* pml4;

void indexer(void* address, uint16_t* pt_i, uint16_t* pd_i, uint16_t* pdp_i, uint16_t* pml4_i);
void map_page(void* virtual_address, void* physical_address, uint16_t flags);
void map_region(void* virtual_address, void* physical_address, uint64_t pages, uint16_t flags);
