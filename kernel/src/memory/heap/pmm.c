#include <memory/heap/pmm.h>

// 512 byte egy saját mmap-nek
new_mmap_entry_t new_mmap[64];
// Mennyi bejegyzés van az új mmap-ben
u8 entries;

bitmap_t pmm_bm;
// Ideiglenes, majd át lesz a bitmap
// költöztetve a heap-be
u8 tmp_bitmap[0x1000];

void pmm_init(mb_tag_memmap_t* mmap) {
	// Meg kell találni a szabad szegmenseket
	u8 num_entries = mmap->base.size / mmap->entry_size;

	u8 mmap_entry = 0;
	for (u8 i = 0; i < num_entries; i++) {
		new_mmap[mmap_entry].addr = mmap->entries[i].addr;
		new_mmap[mmap_entry].length = mmap->entries[i].length;
		new_mmap[mmap_entry].type = mmap->entries[i].type;

		if (i == num_entries-1) {
			mmap_entry++;
			continue;
		}

		if ((mmap->entries[i].addr + mmap->entries[i].length) < (mmap->entries[i+1].addr)) {
			mmap_entry++;
			new_mmap[mmap_entry].addr = mmap->entries[i].addr + mmap->entries[i].length;
			new_mmap[mmap_entry].length = (mmap->entries[i+1].addr) - (mmap->entries[i].addr + mmap->entries[i].length);
			// A 60-as típus csak a MB mmap-jéből hiányzó területeket takarja le
			new_mmap[mmap_entry].type = 60;
		}

		mmap_entry++;
	}
	entries = mmap_entry;

	pmm_bm.addr = (u64)tmp_bitmap;
	pmm_bm.size = 0x1000;
	bm_init(&pmm_bm);

	// A kernel, multiboot és a bitmap page-eit meg kell jelölni használtnak
	pmm_set_used(info->kernel_addr, 1);
	pmm_set_used(info->mb_hdr_addr, 1);
}

// Page-et ezen a címen beállít used-ra
void pmm_set_used(u64 addr, u8 used) {
	if (pmm_reverse_translate(addr) == -1ULL)
		return;
	bm_set(&pmm_bm, pmm_reverse_translate(addr)/PAGESIZE, used);
}

// A heap sajnos szét van szórva a memóriában, ezért
// lesz egy "pseudo" address space, amit a bitmap fog használni
// (tehát hogy néznének ki a heap címek ha a heap 0-nál kezdődne,
// és nem lenne félbeszakítva).
// Ezt a pseudo címet fordítja át az igazi, fizikai címre
u64 pmm_translate(u64 pseudo_addr) {
	u64 copy = pseudo_addr;

	for (u8 i = 0; i < entries; i++) {
		if (new_mmap[i].type != 1)
			continue;

		if (new_mmap[i].length > copy) {
			return new_mmap[i].addr+copy;
		} else {
			copy -= new_mmap[i].length;
		}
	}

	return -1;
}

// Fizikai címet fordít le pseudo címre
u64 pmm_reverse_translate(u64 physical_addr) {
	u64 copy = physical_addr;

	for (u64 i = 0; i < entries; i++) {
		if (new_mmap[i].type != 1) {
			copy -= new_mmap[i].length;
		} else {
			if (copy < (new_mmap[i].addr+new_mmap[i].length-1)) {
				return copy;
			}
		}
	}

	return -1;
}

u64 pmm_alloc_page(void) {
	return pmm_translate(bm_set_next_free(&pmm_bm)*PAGESIZE);
}
