#pragma once

#pragma once

#include <types.hh>

namespace arch {
	enum struct FLAGS : u64 {
		// Saját flagek a 2M és 1G-osok megkülönböztetésére
		s1G	=		(1ULL << 31),
		s2M	=		(1ULL << 30),

		HUGE =		0b10000000,
		USER = 		0b00000100,
		RW =		0b00000010,
		PRESENT =	0b00000001,

		UDATA = USER | RW | PRESENT,
		KDATA = RW | PRESENT,

		PAT0_SHIFT = 3,
		PAT1_SHIFT = 4,
		PAT2_SHIFT = 12,
	};

	#define ADDR_PTI(a) ((((u64)a) >> 12ULL) & 511ULL)
	#define ADDR_PDI(a) ((((u64)a) >> 21ULL) & 511ULL)
	#define ADDR_PDPI(a) ((((u64)a) >> 30ULL) & 511ULL)
	#define ADDR_PML4I(a) ((((u64)a) >> 39ULL) & 511ULL)

	punion page_table_entry {
		u16 flags : 12;
		u64 addr;
	};

	pstruct page_table {
		page_table_entry entries[512];
	};

	extern page_table* pml4;

	void paging_init(void);
	u64 paging_lookup(u64 virt);
	void map_page(u64 virt, u64 phys, u32 flags);
}
