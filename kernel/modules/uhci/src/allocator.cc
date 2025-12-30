#include "allocator.hh"

#include <mm/vmm.hh>
#include <arch/amd64/paging.hh>

static bitmap_t bm;
static u64 pool = 0;

// Mennyi byte egy allokáció: egy TD 32 byte,
// viszont a felső 16 byte opcionális, így lehet elég a 16 byte-os egységméret
static constexpr u32 bytesperunit = 32;

static constexpr u32 num_units = pmm_pagesize / bytesperunit;

static void pool_init() {
	pool = uhci_alloc_page();
	bm.init(kmalloc(num_units * 8), num_units);
}

uhci_td_t* uhci_alloc_td() {
	if (!pool)
		pool_init();

	u64 a = (u64)pool + bm.find_and_set() * bytesperunit;
	if constexpr(DBG) assert((a >> 32) == 0);
	return VIRTUAL((uhci_td_t*)a);
}

// 16 byte aligned
uhci_qh_t* uhci_alloc_qh() {
	if (!pool)
		pool_init();

	u64 a = (u64)pool + bm.find_and_set() * bytesperunit;
	if constexpr(DBG) assert((a >> 32) == 0);
	return VIRTUAL((uhci_qh_t*)a);
}

u32 uhci_alloc_page() {
	u64 a = (u64)pmm_alloc();
	if constexpr(DBG) assert(!(paging_lookup(a) >> 32));
	return (u32)PHYSICAL(a);
}

void uhci_free(void *p)	{
	u64 idx = (PHYSICAL((u64)p) - (u64)pool) / bytesperunit;
	if constexpr (DBG) assert((PHYSICAL((u64)p) - (u64)pool) % bytesperunit == 0);
	bm.set(idx, false);
}
