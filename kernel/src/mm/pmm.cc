#include <mm/pmm.hh>
#include <arch/limine.hh>
#include <arch/arch.hh>
#include <util/bitmap.hh>
#include <util/async.hh>
#include <gfx/console.hh>
#include <cppcompat.hh>

#include <mm/pmm4g.hh>

__attribute__((used, section(".limine_requests")))
static volatile limine_memmap_request mm_req = {
	.id = LIMINE_MEMMAP_REQUEST,
	.revision = 0,
	.response = nullptr,
};

u64 pmm_freemem = 0, pmm_usedmem = 0, pmm_reservedmem = 0;
u8 bitmapStorage[sizeof(bitmap_t)];
bitmap_t* pmm_bm;
void* pmm_heap_base;
u64 pmm_heap_size = 0;
mutex pmm_m;

void pmm_init() {
	limine_memmap_response* r = mm_req.response;

	mmap = r;

	pmm_bm = new (bitmapStorage) bitmap_t;

	u64 pmm4g_base = 0, pmm4g_size = 0;

	for (u32 i = 0; i < r->entry_count; i++) {
		switch (r->entries[i]->type) {
			// nem tudom miért de 12 gigányi foglalt terület van qemuban
			case MMAP_TYPES::LIMINE_MEMMAP_RESERVED:
				break;

			case MMAP_TYPES::LIMINE_MEMMAP_ACPI_RECLAIMABLE:
			case MMAP_TYPES::LIMINE_MEMMAP_ACPI_NVS:
			case MMAP_TYPES::LIMINE_MEMMAP_BAD_MEMORY:
			case MMAP_TYPES::LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
			case MMAP_TYPES::LIMINE_MEMMAP_FRAMEBUFFER:
				// pmm_reservedmem += r->entries[i]->length;
				break;
			case MMAP_TYPES::LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
				// pmm_usedmem += r->entries[i]->length;
				break;
			case MMAP_TYPES::LIMINE_MEMMAP_USABLE:
				u64 base = r->entries[i]->base;
				u64 len = r->entries[i]->length;

				if (len > pmm_heap_size) {
					pmm_heap_size = align_down(len, pmm_pagesize);
					pmm_heap_base = (void*)base;
				}
				pmm_freemem = pmm_heap_size;
				break;
			break;
		}
	}

	for (u32 i = 0; i < r->entry_count; i++) {
		if (r->entries[i]->type == MMAP_TYPES::LIMINE_MEMMAP_USABLE) {
			u64 base = r->entries[i]->base;
			u64 len = r->entries[i]->length;

			if (base < 0xffffffffULL && base != (u64)pmm_heap_base) {
				u64 end = min(0xffffffffULL + 1, base + len);
				// This cuts 'length' off if it crosses the 4G boundary
				u64 truesize = end - base;
				if (truesize > pmm4g_size) {
					pmm4g_base = base;
					pmm4g_size = truesize;
				}
			}
		}
	}

	// if (!pmm4g_base)
	// 	sprintk("No suitable pmm4g heap!\n\r");
	// else
	// 	sprintk("pmm4g heap: %p %d\n\r", pmm4g_base, pmm4g_size);
	// pause();
	
	pmm_heap_base = (void*)align((u64)pmm_heap_base, pmm_pagesize);

	pmm_bm->init(VIRTUAL((u64*)pmm_heap_base), pmm_heap_size / pmm_pagesize);
	// Le kell foglalni a page-eket amikben a bitmap van
	u64 buffer_size = pmm_heap_size / pmm_pagesize / 8 / pmm_pagesize + 1;
	for (u32 i = 0; i < buffer_size; i++) {
		pmm_bm->set(i, true);
		pmm_usedmem += pmm_pagesize;
		pmm_freemem -= pmm_pagesize;
	}
	sprintk("init pmm heap at %p size %llx (%lld MiB)\n\r", pmm_heap_base, pmm_heap_size, bytes2mibs(pmm_heap_size));
	sprintk("bits at %p", pmm_bm->buffer);

	pmm4g_init(pmm4g_base, pmm4g_size);
}

void* pmm_alloc(u64 size) {
	// pmm_m.lock();
	assert(size <= pmm_pagesize);

	pmm_usedmem += pmm_pagesize;
	pmm_freemem -= pmm_pagesize;

	// if (!pmm_freemem)
	// 	fatal("pmm_freemem ran out");

	u64 bit = pmm_bm->find_and_set();
	// sprintk("turipped %d\n\r", bit);
	// if (bit == -1ULL) {
	// 	fatal(
	// 		"Out of memory! free: %lld KiB, used: %lld KiB, all: %lld KiB",
	// 		bytes2kibs(pmm::pmm_freemem),
	// 		bytes2kibs(pmm::pmm_usedmem),
	// 		bytes2kibs(pmm::pmm_heap_size)
	// 	);
	// }
	// pmm_m.unlock();
	return VIRTUAL((void*)((u64)pmm_heap_base + bit * pmm_pagesize));
}

void pmm_free(void* p) {
	// lockguard g(pmm_m);
	p = PHYSICAL(p);
	pmm_usedmem -= pmm_pagesize;
	pmm_freemem += pmm_pagesize;
	u64 idx = ((u64)p - (u64)pmm_heap_base) / pmm_pagesize;
	if constexpr (DBG) assert(((u64)p - (u64)pmm_heap_base) % pmm_pagesize == 0);
	pmm_bm->set(idx, false);
}
