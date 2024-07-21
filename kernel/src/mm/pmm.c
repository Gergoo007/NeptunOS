#include <mm/pmm.h>
#include <util/bootboot.h>
#include <gfx/console.h>

#include <util/bitmap.h>

u64 heap_base;
u64 heap_size;

bitmap pmm_bm;

void pmm_init() {
	typeof(*bootboot.mmap_entries)* entry = bootboot.mmap_entries;
	u32 num_entries = (bootboot.size - 128) / 16;

	for (u32 i = 0; i < num_entries; i++, entry++) {
		if (!mmap_free(entry)) continue;

		// 16G van identity mappelve
		if (mmap_ptr(entry) > 16ULL*1024*1024*1024) continue;

		if (heap_size < mmap_size(entry)) {
			heap_base = mmap_ptr(entry);
			heap_size = mmap_size(entry);
		}
	}

	printk("Valasztott heap: %dG @ %p\n", heap_size / 1024 / 1024 / 1024, heap_base);

	pmm_bm.base = (u8*)heap_base;
	pmm_bm.size = heap_size / 0x1000 / 8 + 1;

	pmm_bm.size |= 0b111;
	pmm_bm.size++;

	bm_init(&pmm_bm);

	// A bitmap-et nem szabad átírni!
	for (u32 i = 0; i < pmm_bm.size / 0x1000 + 1; i += 0x1000) {
		bm_set(&pmm_bm, i, 1);
	}
}

void* pmm_alloc() {
	return (void*)heap_base + bm_alloc(&pmm_bm)*0x1000;
}

void pmm_free(void* p) {
	bm_set(&pmm_bm, ((u64)p - heap_base) / 0x1000, 0);
}
