#include <memory/heap/vmm.h>

// VMM heap rögtön a kernel után következik, ami általában 0xfffffff800200000

bitmap_t vmm_bm;

void vmm_init(void) {
	// VMM-hez a bitmap 8192 page = 32 MiB méretű (kurva nagy, cserébe 1 TiB-nyi helyet tud kezelni)
	// A bitmap 0xFFFFFFFFFD000000-nél fog kezdődni
	// Először csak a bitmap első page-ét mappelem, így 4 KiB segítségével
	// 128 MiB helyet lehet számon tartani
	const void* bm_start = (void*) 0xFFFFFFFFFD000000;
	// for (u16 i = 0; i < 8192; i++) {
	u64 phys = (u64)pmm_alloc_page();
	map_page((u64)bm_start, phys, 0);

	vmm_bm.addr = (u64)bm_start;
	vmm_bm.size = 0x1000;

	bm_init(&vmm_bm);

	// kernel-t used-nak kell megjelölni
	u64 kpages = (&KERNEL_END - &KERNEL_START + 0x200000) / PAGESIZE;
	for (u16 i = 0; i < kpages; i++) {
		bm_set(&vmm_bm, i, 1);
	}
}

void* request_page(void) {
	u64 addr = bm_set_next_free(&vmm_bm) * PAGESIZE + 0xfffffff800000000;
	map_page(addr, (u64)pmm_alloc_page(), 0);
	return (void*)addr;
}

void free_page(void* addr) {
	// addr utolsó 16 bitének eltávolítása
	addr = (void*) ((u64)addr & ~(0xfff));
	bm_set(&vmm_bm, ((u64)addr - 0xfffffff800000000) / PAGESIZE, 0);
	unmap_page((u64)addr);
}

void* vmalloc(void) {
	return (void*) (bm_set_next_free(&vmm_bm) * PAGESIZE + 0xfffffff800000000);
}
