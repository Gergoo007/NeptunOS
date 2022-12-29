#pragma once

#include <stdint.h>

typedef struct pml_entry {
	uint8_t present : 1;
    uint8_t read_write : 1;
    uint8_t user_super : 1;
    uint8_t write_through : 1;
    uint8_t cache_disabled : 1;
    uint8_t accessed : 1;
    uint8_t ignore0 : 1; 
    uint8_t larger_pages : 1;
    uint8_t ignore1 : 1;
    uint8_t available : 3;
    uint64_t address : 52;
} __attribute__((packed)) pml_entry; // if it doesn't work: add align(0x1000)

typedef struct page_map_level {
	pml_entry entries[512];
} __attribute__((packed)) page_map_level;  // if it doesn't work: add align(0x1000)

extern page_map_level* pml4;

void indexer(void* address, uint16_t* pt_i, uint16_t* pd_i, uint16_t* pdp_i, uint16_t* pml4_i);
void map_address(void* virtual_address, void* physical_address);
