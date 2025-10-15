#pragma once

#pragma once

#include <types.hh>

namespace arch {
	struct MFLAGS {
		// Saját flagek a 2M és 1G-osok megkülönböztetésére
		static constexpr u64 s1G	=		(1ULL << 31);
		static constexpr u64 s2M	=		(1ULL << 30);

		static constexpr u64 HUGE =			0b10000000;
		static constexpr u64 USER = 		0b00000100;
		static constexpr u64 RW =			0b00000010;
		static constexpr u64 PRESENT =		0b00000001;

		static constexpr u64 UDATA =		USER | RW | PRESENT;
		static constexpr u64 KDATA =		RW | PRESENT;
	};

	struct MCACHE {
		static constexpr u32 WB = 0;
		static constexpr u32 WT = 1;
		static constexpr u32 UC_WC = 2;
		static constexpr u32 UC = 3;
		static constexpr u32 WP = 4;
		static constexpr u32 WC = 5;
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

	u64 paging_lookup(u64 virt);
	void map_page(u64 virt, u64 phys, u32 flags, u32 cache = MCACHE::WB);
	void check_page(u64 addr);
}
