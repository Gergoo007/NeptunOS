#pragma once

#include <neptunos/config/attributes.h>
#include <neptunos/libk/stdint.h>

#define bytes_kib(bytes) bytes/1024
#define bytes_mib(bytes) bytes_kib(bytes)/1024
#define bytes_gib(bytes) bytes_mib(bytes)/1024
#define bytes_tib(bytes) bytes_gib(bytes)/1024

#define bytes_kb(bytes) bytes/1000
#define bytes_mb(bytes) bytes_kb(bytes)/1000
#define bytes_gb(bytes) bytes_mb(bytes)/1000
#define bytes_tb(bytes) bytes_gb(bytes)/1000

#define kib_bytes(kibis) kibis*1024
#define mib_bytes(mebis) kib_bytes(mebis)*1024
#define gib_bytes(gibis) mib_bytes(gibis)*1024
#define tib_bytes(tibis) gib_bytes(tibis)*1024

#define kb_bytes(kilos) bytes*1000
#define mb_bytes(megas) kb_bytes(megas)*1000
#define gb_bytes(gigas) mb_bytes(gigas)*1000
#define tb_bytes(terras) gb_bytes(terras)*1000

typedef struct bitmap_t {
	void* address;
	uint64_t size;
} bitmap_t;

#include <neptunos/libk/stdall.h>

typedef struct mem_block_hdr_t {
	uint16_t pid;
	uint64_t pages;
} mem_block_hdr_t;

void map_memory();

void init_bitmap(bitmap_t* bm);
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

extern uint64_t total_mem;
extern uint64_t free_mem;
extern uint64_t reserved_mem;
extern uint64_t used_mem;
extern uint64_t num_pages;

extern void* biggest_conv_mem;
extern uint64_t biggest_conv_mem_size;
