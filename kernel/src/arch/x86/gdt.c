#include <arch/x86/gdt.h>

gdt* _gdt;

void gdt_add_tss(tss* _tss) {
	_gdt->tss2.base0 = (u64)_tss;
	_gdt->tss2.base1 = (u64)_tss >> 16;
	_gdt->tss2.base2 = (u64)_tss >> 24;
	_gdt->tss1.base3 = (u64)_tss >> 32;

	// _gdt->tss2.access_byte = 0x89;
	_gdt->tss2.access_byte = 0b10001001;
	_gdt->tss2.dpl = 0;
	_gdt->tss2.avl = 0;
	_gdt->tss2.longmode = 1;
	_gdt->tss2.big = 0;
	_gdt->tss2.granularity = 0;

	u32 limit = sizeof(tss) - 1; // Bitmap beletartozik?
	printk("Limit: %02x\n", limit);

	_gdt->tss2.limit0 = limit & 0xf;
	_gdt->tss2.limit1 = limit >> 4;
}

void gdt_init(void) {
	_gdt = pmm_alloc_page();

	memset(_gdt, 0, sizeof(gdt));
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
	_gdt->ucode.limitl = 0xFFFF;
	_gdt->ucode.basel = 0;
	_gdt->ucode.accessed = 0;
	_gdt->ucode.readable = 1;
	_gdt->ucode.conforming = 0;
	_gdt->ucode.executable = 1;
	_gdt->ucode.type = 1;
	_gdt->ucode.dpl = 3;
	_gdt->ucode.present = 1;
	_gdt->ucode.limith = 0xF;
	_gdt->ucode.avl = 1;
	_gdt->ucode.longmode = 1;
	_gdt->ucode.big = 0;
	_gdt->ucode.granularity = 1;
	_gdt->ucode.baseh = 0;

	_gdt->udata = _gdt->ucode;
	_gdt->udata.executable = 0;

	gdtr _gdtr = {
		.limit = sizeof(gdt)-1,
		.base = (u64)_gdt,
	};

	gdt_load(&_gdtr);
}
