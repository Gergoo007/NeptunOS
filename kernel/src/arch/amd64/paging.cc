#include <arch/amd64/paging.hh>
#include <util/mem.hh>
#include <mm/pmm.hh>

namespace arch {
	page_table* pml4 = nullptr;

	u64 paging_lookup(u64 virt) {
		if (!pml4) {
			asm volatile ("movq %%cr3, %0" : "=a"(pml4));
			pml4 = VIRTUAL(pml4);
		}

		if (!(pml4->entries[ADDR_PML4I(virt)].flags & 1))
			return -1;
		page_table* pdp = (page_table*)VIRTUAL(pml4->entries[ADDR_PML4I(virt)].addr & ~0x0fff);
		if (!(pdp->entries[ADDR_PDPI(virt)].flags & 1))
			return -1;
		page_table* pd = (page_table*)VIRTUAL(pdp->entries[ADDR_PDPI(virt)].addr & ~0x0fff);
		if (!(pd->entries[ADDR_PDI(virt)].flags & 1))
			return -1;

		if (pdp->entries[ADDR_PDPI(virt)].flags & MFLAGS::HUGE) {
			return (u64)pd + (virt & ((1 << 30)-1));
		} else {
			page_table* pt = (page_table*)VIRTUAL(pd->entries[ADDR_PDI(virt)].addr & ~0x0fff);
			if (pd->entries[ADDR_PDI(virt)].flags & MFLAGS::HUGE) {
				return PHYSICAL((u64)pt + (virt & 0x1fffff));
			} else {
				if (!(pt->entries[ADDR_PTI(virt)].flags & 1))
					return -1;
				return (pt->entries[ADDR_PTI(virt)].addr & ~0x0fff) + (virt & 0x0fff);
			}
		}
	}

	void map_page(u64 virt, u64 phys, u32 flags, u32 cache) {
		if (!pml4) {
			asm volatile ("movq %%cr3, %0" : "=a"(pml4));
			pml4 = VIRTUAL(pml4);
		}

		u16 patbits4k = ((cache & 1) << 3) | (((cache >> 1) & 1) << 4) | (((cache >> 2) & 1) << 7);
		u16 patbits2m = ((cache & 1) << 3) | (((cache >> 1) & 1) << 4) | (((cache >> 2) & 1) << 12);

		page_table* pdp;
		page_table* pd;
		page_table* pt;

		page_table_entry* entry;

		u8 size;
		if (flags & MFLAGS::s2M) {
			phys &= ~((1ULL << 21)-1);
			virt &= ~((1ULL << 21)-1);
			size = 1;
		} else if (flags & MFLAGS::s1G) {
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
		if (entry->flags & MFLAGS::PRESENT) {
			pdp = (page_table*) ((pml4->entries[ADDR_PML4I(virt)].addr & ~0x0fff) | 0xffff800000000000ULL);
			if (flags & MFLAGS::USER)
				pml4->entries[ADDR_PML4I(virt)].flags |= MFLAGS::USER;
		} else {
			pdp = (page_table*)pmm::alloc();
			memset(pdp, 0, 0x1000);
			pml4->entries[ADDR_PML4I(virt)].addr = (u64)pdp & ~0xffff800000000000ULL;
			pml4->entries[ADDR_PML4I(virt)].flags = flags;
		}

		entry = &pdp->entries[ADDR_PDPI(virt)];
		if (size == 2) { // 1G page
			entry->addr = phys;
			entry->flags = flags | MFLAGS::HUGE | patbits2m;
			return;
		} else {
			if (entry->flags & MFLAGS::PRESENT) {
				pd = (page_table*) ((pdp->entries[ADDR_PDPI(virt)].addr & ~0x0fff) | 0xffff800000000000ULL);
				if (flags & MFLAGS::USER)
					pdp->entries[ADDR_PDPI(virt)].flags |= MFLAGS::USER;
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
			entry->flags = flags | MFLAGS::HUGE | patbits2m;
			return;
		} else { // 4K page
			if (entry->flags & MFLAGS::PRESENT) {
				pt = (page_table*) ((pd->entries[ADDR_PDI(virt)].addr & ~0x0fff) | 0xffff800000000000ULL);
				if (flags & MFLAGS::USER)
					pd->entries[ADDR_PDI(virt)].flags |= MFLAGS::USER;
			} else {
				pt = (page_table*)pmm::alloc();
				memset(pt, 0, 0x1000);
				pd->entries[ADDR_PDI(virt)].addr = (u64)pt & ~0xffff800000000000ULL;
				pd->entries[ADDR_PDI(virt)].flags = flags;
			}

			entry = &pt->entries[ADDR_PTI(virt)];
			entry->addr = phys;
			entry->flags = flags | patbits4k;
		}

		asm volatile ("invlpg (%0)" :: "r"(virt));
	}

	void check_page(u64 addr) {
		if (arch::paging_lookup(addr) == (u64)-1) {
			arch::map_page(addr, PHYSICAL(addr), 0b11, arch::MCACHE::UC);
		}
	}
}
