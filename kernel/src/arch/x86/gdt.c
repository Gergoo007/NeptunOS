#include <arch/x86/gdt.h>

void gdt_init(void) {
	gdt_t* gdt = pmm_alloc_page();

	memset(&(gdt->kcode), 0, sizeof(gdt_entry_t));
	gdt->kcode.readable = 1;
	gdt->kcode.executable = 1;
	gdt->kcode.type = 1;
	gdt->kcode.dpl = 0;
	gdt->kcode.present = 1;
	gdt->kcode.longmode = 1;
	gdt->kcode.granularity = 1;

	gdt->kdata = gdt->kcode;
	gdt->kdata.executable = 0;

	gdt->ucode = gdt->kcode;
	gdt->udata = gdt->kdata;

	memset(&(gdt->knull), 0, sizeof(gdt_entry_t));
	memset(&(gdt->unull), 0, sizeof(gdt_entry_t));

	gdtr_t gdtr = {
		.limit = sizeof(gdt_t)-1,
		.base = (u64)gdt,
	};

	gdt_load(&gdtr);
}
