#include <mm/pmm.hh>
#include <arch/limine.hh>
#include <arch/arch.hh>
#include <util/bitmap.hh>
#include <util/async.hh>
#include <gfx/console.hh>
#include <cppcompat.hh>

__attribute__((used, section(".limine_requests")))
static volatile limine_memmap_request mm_req = {
	.id = LIMINE_MEMMAP_REQUEST,
	.revision = 0,
	.response = nullptr,
};

u64 pmm_freemem = 0, pmm_usedmem = 0, pmm_reservedmem = 0;
static u8 bitmapStorage[sizeof(Bitmap)];
Bitmap* pmm_bm;
MutexSimple pmm_mutex;

// Watermark min. méret
constexpr u32 WM_SIZE = mibs2bytes(64);

void pmm_init() {
	limine_memmap_response* r = mm_req.response;
	mmap = r;
	pmm_bm = new (bitmapStorage) Bitmap;

	// {
	// 	u64 entry_count = r->entry_count;
	// 	while (r->entries[entry_count - 1]->type == MMAP_TYPES::LIMINE_MEMMAP_RESERVED) {
	// 		entry_count--;
	// 	}

	// 	u64 total = r->entries[entry_count - 1]->base + r->entries[entry_count - 1]->length;
	// 	total = align(total, PMM_PAGESIZE);

	// 	const u64 bitmap_bytes = total / PMM_PAGESIZE / 8;
	// 	bool found = false;
	// 	for (u32 i = 0; i < entry_count; i++) {
	// 		if (r->entries[i]->type == MMAP_TYPES::LIMINE_MEMMAP_USABLE && r->entries[i]->length >= bitmap_bytes + WM_SIZE) {
	// 			extern u64 wm_cursor;
	// 			wm_cursor = VIRTUAL(r->entries[i]->base + bitmap_bytes);
	// 			extern u64 wm_free;
	// 			wm_free = WM_SIZE;

	// 			pmm_bm->init((void*)VIRTUAL(r->entries[i]->base), total / PMM_PAGESIZE);
	// 			for (u64 bytes = 0; bytes < bitmap_bytes + WM_SIZE; bytes += PMM_PAGESIZE) {
	// 				pmm_bm->set((r->entries[i]->base + bytes) / PMM_PAGESIZE, true);
	// 			}

	// 			found = true;

	// 			break;
	// 		}
	// 	}
	// 	if (!found) {
	// 		fatal("Heap of size %llu MiBs not found! Total mem detected is %llu MiB", bytes2mibs(bitmap_bytes + WM_SIZE), bytes2mibs(total));
	// 	}
		
	// 	for (u32 i = 0; i < entry_count; i++) {
	// 		const auto& e = r->entries[i];
	// 		const auto& next = r->entries[i + 1];

	// 		sprintk("[%d] %p - %p\r\n", e->type, e->base, e->base + e->length);

	// 		if (r->entries[i]->type != MMAP_TYPES::LIMINE_MEMMAP_USABLE) {
	// 			for (u64 bytes = 0; bytes < r->entries[i]->length; bytes += PMM_PAGESIZE) {
	// 				pmm_bm->set((r->entries[i]->base + bytes) / PMM_PAGESIZE, true);
	// 			}

	// 			pmm_reservedmem += r->entries[i]->length;
	// 		} else {
	// 			pmm_freemem += r->entries[i]->length;
	// 		}

	// 		// A lukakat is használhatalannak kell jelölni, mert ezek a faszszopók nem tudnak normális mmapot adni
	// 		if (i != entry_count - 1) {
	// 			if (e->base + e->length < next->base) {
	// 				const u64 bytes = next->base - (e->base + e->length);
	// 				for (u64 pages = 0; pages < align(bytes, PMM_PAGESIZE); pages += PMM_PAGESIZE) {
	// 					pmm_bm->set((e->base + e->length + pages) / PMM_PAGESIZE, true);
	// 				}
	// 			}
	// 		}
	// 	}
	// }

	const auto& last = r->entries[r->entry_count - 1]->type == MMAP_TYPES::LIMINE_MEMMAP_RESERVED ? r->entries[r->entry_count - 2] : r->entries[r->entry_count - 1];
	const u64 total = last->base + last->length;
	const u64 bm_size = align(total, PMM_PAGESIZE) / PMM_PAGESIZE;
	const u64 bm_bytes = bm_size / 8;

	bool found = false;

	for (u32 i = 0; i < r->entry_count; i++) {
		const auto& e = r->entries[i];

		if (e->type == MMAP_TYPES::LIMINE_MEMMAP_USABLE && e->length >= bm_bytes + WM_SIZE) {
			extern u64 wm_cursor;
			wm_cursor = VIRTUAL(e->base);
			extern u64 wm_free;
			wm_free = WM_SIZE;

			pmm_bm->init(VIRTUAL((void*)(e->base + WM_SIZE)), bm_size, true);

			found = true;

			break;
		}
	}

	if (!found) {
		fatal("No good candidate found for bitmap and WM alloc!");
	}

	for (u32 i = 0; i < r->entry_count; i++) {
		const auto& e = r->entries[i];
		if (e->type != MMAP_TYPES::LIMINE_MEMMAP_USABLE) continue;

		pmm_freemem += e->length;

		const u64 base = align(e->base, PMM_PAGESIZE);
		for (u64 j = 0; j < align_down(e->length, PMM_PAGESIZE); j += PMM_PAGESIZE) {
			pmm_bm->set((base + j) / PMM_PAGESIZE, false);
		}

		u64 bytes = 0;

		if (e->base + WM_SIZE == PHYSICAL((u64)pmm_bm->buffer)) {
			for (u64 j = 0; j < bm_size + WM_SIZE; j += PMM_PAGESIZE) {
				pmm_bm->set((e->base + j) / PMM_PAGESIZE, true);
				bytes += PMM_PAGESIZE;
			}
		}

		sprintk("set %lld kibs; totla %lld bm_size %lld\r\n", bytes2kibs(bytes), bytes2mibs(total), bytes2kibs(bm_size));
	}

	// Null pointerek ne legyenek azért
	pmm_bm->set(0, true);
}

void* pmm_alloc(u64 size) {
	pmm_mutex.lock();

	size = align(size, PMM_PAGESIZE);

	pmm_usedmem += size;
	pmm_freemem -= size;

	u64 bit = pmm_bm->find_and_set_multiple(size / PMM_PAGESIZE, gibs2bytes(4) / PMM_PAGESIZE);
	if (bit == -1ull)
		fatal("PMM failed to allocate %llu bytes", size);

	pmm_mutex.unlock();
	return VIRTUAL((void*)(bit * PMM_PAGESIZE));
}

void* pmm_alloc4g(u64 size) {
	pmm_mutex.lock();

	size = align(size, PMM_PAGESIZE);

	pmm_usedmem += size;
	pmm_freemem -= size;

	u64 bit = pmm_bm->find_and_set_multiple(size / PMM_PAGESIZE);
	if (bit == -1ull)
		fatal("PMM failed to allocate %llu bytes", size);

	pmm_mutex.unlock();
	return VIRTUAL((void*)(bit * PMM_PAGESIZE));
}

void pmm_free(void* p) {
	// fatal("unimpl");

	// pmm_mutex.lock();

	// p = PHYSICAL(p);
	// pmm_usedmem -= PMM_PAGESIZE;
	// pmm_freemem += PMM_PAGESIZE;
	// u64 idx = (u64)p / PMM_PAGESIZE;
	// if constexpr (DBG) assert((u64)p % PMM_PAGESIZE == 0);
	// pmm_bm->set(idx, false);

	// pmm_mutex.unlock();
}
