#pragma once

#pragma once

#include <types.hh>

typedef enum MFLAGS {
	// Saját flagek a 2M és 1G-osok megkülönböztetésére, meg a NX-ra
	s1G	=		(1ULL << 31),
	s2M	=		(1ULL << 30),
	EXE	=		(1ULL << 32),

	HUGE =			0b10000000,
	USER = 		0b00000100,
	RW =			0b00000010,
	PRESENT =		0b00000001,
	
	KDATA =		RW | PRESENT,
	UDATA =		USER | KDATA,
	KCODE =		EXE | PRESENT,
	UCODE =		USER | KCODE,
} MFLAGS;

typedef enum MCACHE {
	WB = 0,
	WT = 1,
	UC_WC = 2,
	UC = 3,
	WP = 4,
	WC = 5,
} MCACHE;

#define ADDR_PTI(a) ((((u64)a) >> 12ULL) & 511ULL)
#define ADDR_PDI(a) ((((u64)a) >> 21ULL) & 511ULL)
#define ADDR_PDPI(a) ((((u64)a) >> 30ULL) & 511ULL)
#define ADDR_PML4I(a) ((((u64)a) >> 39ULL) & 511ULL)

typedef punion page_table_entry {
	u16 flags : 12;
	u64 addr;
} page_table_entry_t;

typedef pstruct page_table {
	page_table_entry_t entries[512];
} page_table_t;

extern page_table_t* pml4;

u64 paging_lookup(u64 virt);
void map_page(u64 virt, u64 phys, u64 flags, u32 cache = WB);
void check_page(u64 addr, u64 cache);
void check_pages(u64 addr, u64 count);
