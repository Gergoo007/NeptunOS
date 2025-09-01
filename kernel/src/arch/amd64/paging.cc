#include <arch/amd64/paging.hh>
#include <util/mem.hh>
#include <mm/pmm.hh>

namespace arch {
	page_table* pml4 = nullptr;

	u64 paging_lookup(u64 virt) {
		page_table* pdp = (page_table*)VIRTUAL(pml4->entries[ADDR_PML4I(virt)].addr & ~0x0fff);
		page_table* pd = (page_table*)VIRTUAL(pdp->entries[ADDR_PDPI(virt)].addr & ~0x0fff);

		if (pdp->entries[ADDR_PDPI(virt)].flags & (u64)FLAGS::HUGE) {
			return (u64)pd + (virt & ((1 << 30)-1));
		} else {
			page_table* pt = (page_table*)VIRTUAL(pd->entries[ADDR_PDI(virt)].addr & ~0x0fff);
			if (pd->entries[ADDR_PDI(virt)].flags & (u64)FLAGS::HUGE) {
				return PHYSICAL((u64)pt + (virt & 0x1fffff));
			} else {
				return (pt->entries[ADDR_PTI(virt)].addr & ~0x0fff) + (virt & 0x0fff);
			}
		}
	}

	void map_page(u64 virt, u64 phys, u32 flags) {
		if (!pml4) {
			asm volatile ("movq %%cr3, %0" : "=a"(pml4));
			pml4 = VIRTUAL(pml4);
		}

		page_table* pdp;
		page_table* pd;
		page_table* pt;

		page_table_entry* entry;

		u8 size;
		if (flags & (u64)FLAGS::s2M) {
			phys &= ~((1ULL << 21)-1);
			virt &= ~((1ULL << 21)-1);
			size = 1;
		} else if (flags & (u64)FLAGS::s1G) {
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
		if (entry->flags & (u64)FLAGS::PRESENT) {
			pdp = (page_table*) ((pml4->entries[ADDR_PML4I(virt)].addr & ~0x0fff) | 0xffff800000000000ULL);
			if (flags & (u64)FLAGS::USER)
				pml4->entries[ADDR_PML4I(virt)].flags |= (u64)FLAGS::USER;
		} else {
			pdp = (page_table*)pmm::alloc();
			memset(pdp, 0, 0x1000);
			pml4->entries[ADDR_PML4I(virt)].addr = (u64)pdp & ~0xffff800000000000ULL;
			pml4->entries[ADDR_PML4I(virt)].flags = flags;
		}

		entry = &pdp->entries[ADDR_PDPI(virt)];
		if (size == 2) { // 1G page
			entry->addr = phys;
			entry->flags = flags | (u64)FLAGS::HUGE;
			return;
		} else {
			if (entry->flags & (u64)FLAGS::PRESENT) {
				pd = (page_table*) ((pdp->entries[ADDR_PDPI(virt)].addr & ~0x0fff) | 0xffff800000000000ULL);
				if (flags & (u64)FLAGS::USER)
					pdp->entries[ADDR_PDPI(virt)].flags |= (u64)FLAGS::USER;
			} else {
				pd = (page_table*)pmm::alloc();
				memset(pd, 0, 0x1000);
				pdp->entries[ADDR_PDPI(virt)].addr = (u64)pd & ~0xffff800000000000ULL;
				pdp->entries[ADDR_PDPI(virt)].flags = flags;
			}
		}

		entry = &pd->entries[ADDR_PDI(virt)];
		if (size == 1) { // 2M page
			entry->addr = phys;
			entry->flags = flags | (u64)FLAGS::HUGE;
			return;
		} else { // 4K page
			if (entry->flags & (u64)FLAGS::PRESENT) {
				pt = (page_table*) ((pd->entries[ADDR_PDI(virt)].addr & ~0x0fff) | 0xffff800000000000ULL);
				if (flags & (u64)FLAGS::USER)
					pd->entries[ADDR_PDI(virt)].flags |= (u64)FLAGS::USER;
			} else {
				pt = (page_table*)pmm::alloc();
				memset(pt, 0, 0x1000);
				pd->entries[ADDR_PDI(virt)].addr = (u64)pt & ~0xffff800000000000ULL;
				pd->entries[ADDR_PDI(virt)].flags = flags;
			}

			entry = &pt->entries[ADDR_PTI(virt)];
			entry->addr = phys;
			entry->flags = flags;
		}

		asm volatile ("invlpg (%0)" :: "r"(virt));
	}
}
