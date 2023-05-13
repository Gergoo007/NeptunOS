#include "../stdint.h"
#include "../pmm/pmm.h"

// typedef enum {
// 	MAP_FLAG_PRESENT = 			0b000000000001,
// 	MAP_FLAG_RW = 				0b000000000010,
// 	MAP_FLAG_SUPER =			0b000000000100,
// 	MAP_FLAG_WRITE_THROUGH = 	0b000000001000,
// 	MAP_FLAG_CACHE_DISABLE = 	0b000000010000,
// 	MAP_FLAG_ACCESSED = 		0b000000100000,
// 	MAP_FLAG_AVAILABLE = 		0b000001000000,
// 	MAP_FLAG_4MB_PAGES = 		0b000010000000,
// 	MAP_FLAGS_DEFAULTS =		0 | MAP_FLAG_PRESENT | MAP_FLAG_RW,
// 	MAP_FLAGS_IO_DEFAULTS =		0 | MAP_FLAG_PRESENT | MAP_FLAG_RW | MAP_FLAG_CACHE_DISABLE,
// } map_flags_t;

extern const u16 MAP_FLAG_PRESENT;
extern const u16 MAP_FLAG_RW;
extern const u16 MAP_FLAG_SUPER ;
extern const u16 MAP_FLAG_WRITE_THROUGH;
extern const u16 MAP_FLAG_CACHE_DISABLE;
extern const u16 MAP_FLAG_ACCESSED;
extern const u16 MAP_FLAG_AVAILABLE;
extern const u16 MAP_FLAG_4MB_PAGES;
extern const u16 MAP_FLAGS_DEFAULTS;
extern const u16 MAP_FLAGS_IO_DEFAULTS;

typedef struct page_map_entry {
	union {
		struct {
			u8 present 		: 1;
			u8 read_write 		: 1;
			u8 user_super 		: 1;
			u8 write_through 	: 1;
			u8 cache_disabled 	: 1;
			u8 accessed 		: 1;
			u8 ignore0 		: 1;
			u8 larger_pages 	: 1;
			u8 ignore1 		: 1;
			u8 available 		: 3;
			u64 address 		: 48;
			u8 : 4; 	// To make it exactly 64 bits
		};
		struct {
			u16 flags : 12;
			u64 : 48; 	// Address, but it's already present in the other struct
			u8 : 4; 	// To make it exactly 64 bits
		};
	};
} __attribute__((packed)) page_map_entry;

typedef struct page_map_level {
	page_map_entry entries[512];
} __attribute__((packed)) page_map_level;

extern page_map_level* pml4;
void indexer(void* address, u16* pt_i, u16* pd_i, u16* pdp_i, u16* pml4_i);
void map_page(void* virtual_address, void* physical_address, u16 flags);
void map_region(void* virtual_address, void* physical_address, u64 pages, u16 flags);
void setup_paging();
