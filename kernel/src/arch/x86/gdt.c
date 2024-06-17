#include <arch/x86/gdt.h>

void gdt_init(void) {
	gdt* _gdt = pmm_alloc_page();

	memset(&(_gdt->kcode), 0, sizeof(gdt_entry));
	_gdt->kcode.readable = 1;
	_gdt->kcode.executable = 1;
	_gdt->kcode.type = 1;
	_gdt->kcode.dpl = 0;
	_gdt->kcode.present = 1;
	_gdt->kcode.longmode = 1;
	_gdt->kcode.granularity = 1;

	_gdt->kdata = _gdt->kcode;
	_gdt->kdata.executable = 0;

	_gdt->ucode = _gdt->kcode;
	_gdt->udata = _gdt->kdata;

	memset(&(_gdt->knull), 0, sizeof(gdt_entry));
	memset(&(_gdt->unull), 0, sizeof(gdt_entry));

	gdtr _gdtr = {
		.limit = sizeof(gdt)-1,
		.base = (u64)_gdt,
	};

	gdt_load(&_gdtr);
}
