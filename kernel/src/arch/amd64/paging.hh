#pragma once

#pragma once

#include <types.hh>

typedef enum MFLAGS {
	// Saját flagek a 2M és 1G-osok megkülönböztetésére, meg a NX-ra
	s1G	=		(1ULL << 31),
	s2M	=		(1ULL << 30),
	EXE	=		(1ULL << 32),

	HUGE =		0b10000000,
	USER = 		0b00000100,
	RW =		0b00000010,
	PRESENT =	0b00000001,
	
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

pstruct page_table_t;
punion page_table_entry_t {
	u16 flags : 12;
	u64 addr;

	page_table_t* getaddr() { return (page_table_t*)(addr & ~(0x0fffull | (1ull << 63))); }
	page_table_t* getvaddr() { return VIRTUAL(getaddr()); }
};

pstruct page_table_t {
	page_table_entry_t entries[512];
};

extern page_table_t* pml4;

template <typename T>
u64 paging_lookup(page_table_t* cr3, T _virt) {
	u64 virt = (u64)_virt;

	if (!(cr3->entries[ADDR_PML4I(virt)].flags & 1))
		return -1;
	page_table_t* pdp = (page_table_t*)VIRTUAL(cr3->entries[ADDR_PML4I(virt)].addr & ~0x0fff);
	if (!(pdp->entries[ADDR_PDPI(virt)].flags & 1))
		return -1;
	page_table_t* pd = (page_table_t*)VIRTUAL(pdp->entries[ADDR_PDPI(virt)].addr & ~0x0fff);
	if (!(pd->entries[ADDR_PDI(virt)].flags & 1))
		return -1;

	if (pdp->entries[ADDR_PDPI(virt)].flags & HUGE) {
		return ((u64)pd + (virt & ((1 << 30)-1))) & ~(1ull << 63);
	} else {
		page_table_t* pt = (page_table_t*)VIRTUAL(pd->entries[ADDR_PDI(virt)].addr & ~0x0fff);
		if (pd->entries[ADDR_PDI(virt)].flags & HUGE) {
			return (PHYSICAL((u64)pt + (virt & 0x1fffff))) & ~(1ull << 63);
		} else {
			if (!(pt->entries[ADDR_PTI(virt)].flags & 1))
				return -1;
			return ((pt->entries[ADDR_PTI(virt)].addr & ~0x0fff) + (virt & 0x0fff)) & ~(1ull << 63);
		}
	}
}

template <typename T>
u64 paging_lookup(T _virt) {
	u64 virt = (u64)_virt;
	if (!pml4) {
		asm volatile ("movq %%cr3, %0" : "=a"(pml4));
		pml4 = VIRTUAL(pml4);
	}

	return paging_lookup(pml4, virt);
}

template <typename T>
void paging_track(T _virt) {
	u64 virt = (u64)_virt;
	if (!pml4) {
		asm volatile ("movq %%cr3, %0" : "=a"(pml4));
		pml4 = VIRTUAL(pml4);
	}

	auto* pml4ent = &pml4->entries[ADDR_PML4I(virt)];
	report("pml4ent %p @ %p", *(void**)pml4ent, pml4ent);
	auto* pdpent = &pml4ent->getvaddr()->entries[ADDR_PDPI(virt)];
	report("pdpent %p @ %p", *(void**)pdpent, pdpent);
	if (pdpent->flags & HUGE) return;
	auto* pdent = &pdpent->getvaddr()->entries[ADDR_PDI(virt)];
	report("pdent %p @ %p", *(void**)pdent, pdent);
	if (pdent->flags & HUGE) return;
	auto* ptent = &pdent->getvaddr()->entries[ADDR_PTI(virt)];
	report("ptent %p @ %p", *(void**)ptent, ptent);
}

void map_page(page_table_t* cr3, u64 virt, u64 phys, u64 flags, u32 cache = WB);
void map_page(u64 virt, u64 phys, u64 flags, u32 cache = WB);

void check_page(u64 addr, u64 cache);
void check_pages(u64 addr, u64 count);
