#include <arch/x86/gdt.h>
#include <mm/vmm.h>
#include <util/mem.h>

#include <gfx/console.h>

void gdt_init() {
	gdt_entry* gdt = vmm_alloc();
	memset_aligned(gdt, 0, 0x1000 / 8);

	// Null entry
	memset(&gdt[0], 0, sizeof(gdt_entry));

	// Kernel kód
	gdt[1] = (gdt_entry) {
		.base1 = 0,
		.base2 = 0,
		.base3 = 0,

		.limit1 = 0xffff,
		.limit2 = 0xf,

		.access_byte.seg_desc = {
			.conforming = 0,
			.data_code = 1,
			.dpl = 0,
			.exec = 1,
			.present = 1,
			.rw = 1,
		},

		.limit_in_pages = 1,
		.long_mode = 1,
		.size = 0,
	};

	// Kernel adat
	gdt[2] = gdt[1];
	gdt[2].access_byte.seg_desc.exec = 0;

	// Felhasználó kód
	gdt[3] = gdt[1];
	gdt[3].access_byte.seg_desc.dpl = 3;

	// Felhasználó adat
	gdt[4] = gdt[2];
	gdt[3].access_byte.seg_desc.dpl = 3;

	gdt_load(&(gdtr) {
		sizeof(gdt_entry) * 5 - 1, // TSS nélkül
		(u64) gdt,
	});
}
