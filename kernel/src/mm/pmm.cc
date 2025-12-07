#include <mm/pmm.hh>
#include <arch/limine.hh>
#include <arch/arch.hh>
#include <util/bitmap.hh>
#include <gfx/console.hh>
#include <cppcompat.hh>

__attribute__((used, section(".limine_requests")))
static volatile limine_memmap_request mm_req = {
	.id = LIMINE_MEMMAP_REQUEST,
	.revision = 0,
	.response = nullptr,
};

u64 pmm_freemem = 0, pmm_usedmem = 0, pmm_reservedmem = 0;
u8 bitmapStorage[sizeof(bitmap_t)];
bitmap_t* bm;
void* pmm_heap_base;
u64 pmm_heap_size = 0;

void pmm_init() {
	limine_memmap_response* r = mm_req.response;

	mmap = r;

	bm = new (bitmapStorage) bitmap_t;

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
				if (r->entries[i]->length > pmm_heap_size) {
					pmm_heap_size = align_down(r->entries[i]->length, pmm_pagesize);
					pmm_heap_base = (void*)r->entries[i]->base;
				}
				pmm_freemem = pmm_heap_size;
				break;
			break;
		}
	}

	bm->init(VIRTUAL((u64*)pmm_heap_base), pmm_heap_size / pmm_pagesize);
	// Le kell foglalni a page-eket amikben a bitmap van
	u64 buffer_size = pmm_heap_size / pmm_pagesize / 8 / pmm_pagesize + 1;
	for (u32 i = 0; i < buffer_size; i++) {
		bm->set(i, true);
		pmm_usedmem += pmm_pagesize;
		pmm_freemem -= pmm_pagesize;
	}
	sprintk("init pmm heap at %p size %llx (%lld MiB)\n\r", pmm_heap_base, pmm_heap_size, bytes2mibs(pmm_heap_size));
	sprintk("bits at %p", bm->buffer);
}

void* pmm_alloc(u64 size) {
	assert(size <= pmm_pagesize);

	pmm_usedmem += pmm_pagesize;
	pmm_freemem -= pmm_pagesize;

	// if (!pmm_freemem)
	// 	fatal("pmm_freemem ran out");

	u64 bit = bm->find_and_set();
	// if (bit == -1ULL) {
	// 	fatal(
	// 		"Out of memory! free: %lld KiB, used: %lld KiB, all: %lld KiB",
	// 		bytes2kibs(pmm::pmm_freemem),
	// 		bytes2kibs(pmm::pmm_usedmem),
	// 		bytes2kibs(pmm::pmm_heap_size)
	// 	);
	// }
	return VIRTUAL((void*)((u64)pmm_heap_base + bit * pmm_pagesize));
}

void pmm_free(void* p) {
	p = PHYSICAL(p);
	pmm_usedmem -= pmm_pagesize;
	pmm_freemem += pmm_pagesize;
	u64 idx = ((u64)p - (u64)pmm_heap_base) / pmm_pagesize;
	if constexpr (debug) assert(((u64)p - (u64)pmm_heap_base) % pmm_pagesize == 0);
	bm->set(idx, false);
}
