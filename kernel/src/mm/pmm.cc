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

namespace pmm {
	u64 freemem = 0, usedmem = 0, reservedmem = 0;
	u8 bitmapStorage[sizeof(Bitmap)];
	Bitmap* bm;
	void* heap_base;
	u64 heap_size = 0;

	void init() {
		limine_memmap_response* r = mm_req.response;

		machine.mmap = r;

		bm = new (bitmapStorage) Bitmap;

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
					// reservedmem += r->entries[i]->length;
					break;
				case MMAP_TYPES::LIMINE_MEMMAP_EXECUTABLE_AND_MODULES:
					// usedmem += r->entries[i]->length;
					break;
				case MMAP_TYPES::LIMINE_MEMMAP_USABLE:
					if (r->entries[i]->length > heap_size) {
						heap_size = align_down(r->entries[i]->length, pagesize);
						heap_base = (void*)r->entries[i]->base;
					}
					freemem = heap_size;
					break;
				break;
			}
		}

		bm->init(VIRTUAL((u64*)heap_base), heap_size / pagesize);
		// Le kell foglalni a page-eket amikben a bitmap van
		u64 buffer_size = heap_size / pagesize / 8 / pagesize + 1;
		for (u32 i = 0; i < buffer_size; i++) {
			bm->set(i, true);
			usedmem += pagesize;
			freemem -= pagesize;
		}
		sprintk("init pmm heap at %p size %llx (%lld MiB)\n\r", heap_base, heap_size, bytes2mibs(heap_size));
		sprintk("bits at %p\n", bm->buffer);
	}

	void* alloc(u64 size) {
		if (size < pagesize) size = pagesize;
		size = align(size, pagesize);

		usedmem += pagesize;
		freemem -= pagesize;

		// if (!freemem)
		// 	fatal("freemem ran out\n");

		u64 bit = bm->find_and_set();
		// if (bit == -1ULL) {
		// 	fatal(
		// 		"Out of memory! free: %lld KiB, used: %lld KiB, all: %lld KiB\n",
		// 		bytes2kibs(pmm::freemem),
		// 		bytes2kibs(pmm::usedmem),
		// 		bytes2kibs(pmm::heap_size)
		// 	);
		// }
		return VIRTUAL((void*)((u64)heap_base + bit * pagesize));
	}
}
