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
	u64 free = 0, used = 0, reserved = 0;
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
					reserved += r->entries[i]->length;
					break;
				case MMAP_TYPES::LIMINE_MEMMAP_KERNEL_AND_MODULES:
					used += r->entries[i]->length;
					break;
				case MMAP_TYPES::LIMINE_MEMMAP_USABLE:
					if (r->entries[i]->length > heap_size) {
						heap_size = r->entries[i]->length;
						heap_base = (void*)r->entries[i]->base;
					}
					free += r->entries[i]->length;
					break;
				break;
			}
		}

		bm->init(VIRTUAL((u64*)heap_base), heap_size / pagesize);
		// Le kell foglalni a page-eket amikben a bitmap van
		u64 buffer_size = heap_size / pagesize / 8 / pagesize + 1;
		for (u32 i = 0; i < buffer_size; i++) {
			bm->set(i, true);
			used += pagesize;
			free -= pagesize;
		}
	}

	void* alloc(u64 size) {
		if (size < pagesize) size = pagesize;
		size = align(size, pagesize);

		return VIRTUAL((void*)((u64)heap_base + bm->find_and_set() * pagesize));
	}
}
