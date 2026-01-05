#include <arch/amd64/paging.hh>
#include <util/mem.hh>
#include <mm/pmm.hh>

page_table_t* pml4 = nullptr;

u64 paging_lookup(u64 virt) {
	if (!pml4) {
		asm volatile ("movq %%cr3, %0" : "=a"(pml4));
		pml4 = VIRTUAL(pml4);
	}

	if (!(pml4->entries[ADDR_PML4I(virt)].flags & 1))
		return -1;
	page_table_t* pdp = (page_table_t*)VIRTUAL(pml4->entries[ADDR_PML4I(virt)].addr & ~0x0fff);
	if (!(pdp->entries[ADDR_PDPI(virt)].flags & 1))
		return -1;
	page_table_t* pd = (page_table_t*)VIRTUAL(pdp->entries[ADDR_PDPI(virt)].addr & ~0x0fff);
	if (!(pd->entries[ADDR_PDI(virt)].flags & 1))
		return -1;

	if (pdp->entries[ADDR_PDPI(virt)].flags & HUGE) {
		return (u64)pd + (virt & ((1 << 30)-1));
	} else {
		page_table_t* pt = (page_table_t*)VIRTUAL(pd->entries[ADDR_PDI(virt)].addr & ~0x0fff);
		if (pd->entries[ADDR_PDI(virt)].flags & HUGE) {
			return PHYSICAL((u64)pt + (virt & 0x1fffff));
		} else {
			if (!(pt->entries[ADDR_PTI(virt)].flags & 1))
				return -1;
			return (pt->entries[ADDR_PTI(virt)].addr & ~0x0fff) + (virt & 0x0fff);
		}
	}
}

void map_page(u64 virt, u64 phys, u64 flags, u32 cache) {
	if (!pml4) {
		asm volatile ("movq %%cr3, %0" : "=a"(pml4));
		pml4 = VIRTUAL(pml4);
	}

	u16 patbits4k = ((cache & 1) << 3) | (((cache >> 1) & 1) << 4) | (((cache >> 2) & 1) << 7);
	u16 patbits2m = ((cache & 1) << 3) | (((cache >> 1) & 1) << 4) | (((cache >> 2) & 1) << 12);

	page_table_t* pdp;
	page_table_t* pd;
	page_table_t* pt;

	page_table_entry_t* entry;

	u64 exemask = (flags & EXE) > 0 ? 0x0ULL : 0x8000000000000000ULL;

	u8 size;
	if (flags & s2M) {
		phys &= ~((1ULL << 21)-1);
		virt &= ~((1ULL << 21)-1);
		size = 1;
	} else if (flags & s1G) {
		phys &= ~((1ULL << 30)-1);
		virt &= ~((1ULL << 30)-1);
		size = 2;
	} else {
		phys &= ~((1ULL << 12)-1);
		virt &= ~((1ULL << 12)-1);
		size = 0;
	}

	// Custom flagek (bit 12 fölött) eltávolítása
	flags &= (1 << 13)-1;

	entry = &pml4->entries[ADDR_PML4I(virt)];
	if (entry->flags & PRESENT) {
		pdp = (page_table_t*) ((pml4->entries[ADDR_PML4I(virt)].addr & ~0x0fff) | 0xffff800000000000ULL);
		if (flags & USER)
			pml4->entries[ADDR_PML4I(virt)].flags |= USER;
	} else {
		pdp = (page_table_t*)pmm_alloc();
		memset(pdp, 0, 0x1000);
		pml4->entries[ADDR_PML4I(virt)].addr = ((u64)pdp & ~0xffff800000000000ULL) | exemask;
		pml4->entries[ADDR_PML4I(virt)].flags = flags;
	}

	entry = &pdp->entries[ADDR_PDPI(virt)];
	if (size == 2) { // 1G page
		entry->addr = phys | exemask;
		if (!exemask) {
			// Az összes feljebb lévő struktúrán is ki kell kapcsolni az NX bitet
			pml4->entries[ADDR_PML4I(virt)].addr = bitset(pml4->entries[ADDR_PML4I(virt)].addr, 63, 0);
			pdp->entries[ADDR_PDPI(virt)].addr = bitset(pdp->entries[ADDR_PDPI(virt)].addr, 63, 0);
		}

		entry->flags = flags | HUGE | patbits2m;
		return;
	} else {
		if (entry->flags & PRESENT) {
			pd = (page_table_t*) ((pdp->entries[ADDR_PDPI(virt)].addr & ~0x0fff) | 0xffff800000000000ULL);
			if (flags & USER)
				pdp->entries[ADDR_PDPI(virt)].flags |= USER;
		} else {
			pd = (page_table_t*)pmm_alloc();
			memset(pd, 0, 0x1000);
			pdp->entries[ADDR_PDPI(virt)].addr = ((u64)pd & ~0xffff800000000000ULL) | exemask;
			pdp->entries[ADDR_PDPI(virt)].flags = flags;
		}
	}

	entry = &pd->entries[ADDR_PDI(virt)];
	if (size == 1) { // 2M page
		entry->addr = phys | exemask;
		if (!exemask) {
			// Az összes feljebb lévő struktúrán is ki kell kapcsolni az NX bitet
			pml4->entries[ADDR_PML4I(virt)].addr = bitset(pml4->entries[ADDR_PML4I(virt)].addr, 63, 0);
			pdp->entries[ADDR_PDPI(virt)].addr = bitset(pdp->entries[ADDR_PDPI(virt)].addr, 63, 0);
			pd->entries[ADDR_PDI(virt)].addr = bitset(pd->entries[ADDR_PDI(virt)].addr, 63, 0);
		}
		entry->flags = flags | HUGE | patbits2m;
		return;
	} else { // 4K page
		if (entry->flags & PRESENT) {
			pt = (page_table_t*) ((pd->entries[ADDR_PDI(virt)].addr & ~0x0fff) | 0xffff800000000000ULL);
			if (flags & USER)
				pd->entries[ADDR_PDI(virt)].flags |= USER;
		} else {
			pt = (page_table_t*)pmm_alloc();
			memset(pt, 0, 0x1000);
			pd->entries[ADDR_PDI(virt)].addr = ((u64)pt & ~0xffff800000000000ULL) | exemask;
			pd->entries[ADDR_PDI(virt)].flags = flags;
		}

		entry = &pt->entries[ADDR_PTI(virt)];
		entry->addr = phys | exemask;
		if (!exemask) {
			// Az összes feljebb lévő struktúrán is ki kell kapcsolni az NX bitet
			pml4->entries[ADDR_PML4I(virt)].addr = bitset(pml4->entries[ADDR_PML4I(virt)].addr, 63, 0);
			pdp->entries[ADDR_PDPI(virt)].addr = bitset(pdp->entries[ADDR_PDPI(virt)].addr, 63, 0);
			pd->entries[ADDR_PDI(virt)].addr = bitset(pd->entries[ADDR_PDI(virt)].addr, 63, 0);
			pt->entries[ADDR_PTI(virt)].addr = bitset(pt->entries[ADDR_PTI(virt)].addr, 63, 0);
		}
		entry->flags = flags | patbits4k;
	}

	asm volatile ("invlpg (%0)" :: "r"(virt));
}

void check_page(u64 addr, u64 cache) {
	if (paging_lookup(addr) == (u64)-1)
		map_page(addr, PHYSICAL(addr), 0b11, cache);
}

void check_pages(u64 addr, u64 bytes) {
	for (u64 i = 0; i < align(bytes, 0x1000); i += 0x1000)
		check_page(addr + i * 0x1000, WB);
}
