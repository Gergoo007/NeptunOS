#include <arch/amd64/gdt.hh>
#include <mm/vmm.hh>

namespace arch::gdt {
	gdt_entry* gdt;

	extern "C" void gdt_load(gdtr* g);
	void init() {
		gdt = (gdt_entry*)pmm::alloc();
		memset(gdt, 0, 0x1000);

		// Kernel kód (0x08)
		gdt[1].limit1 = 0;
		gdt[1].limit2 = 0;

		gdt[1].base1 = 0;
		gdt[1].base2 = 0;
		gdt[1].base3 = 0;

		gdt[1].access_byte.seg_desc.conforming = 0;
		gdt[1].access_byte.seg_desc.data_code = 1;
		gdt[1].access_byte.seg_desc.dpl = 0;
		gdt[1].access_byte.seg_desc.exec = 1;
		gdt[1].access_byte.seg_desc.present = 1;
		gdt[1].access_byte.seg_desc.rw = 1;

		gdt[1].limit_in_pages = 1;
		gdt[1].long_mode = 1;
		gdt[1].size = 0;
		gdt[1].avl = 0;

		// Kernel adat (0x10)
		gdt[2] = gdt[1];
		gdt[2].access_byte.seg_desc.exec = 0;

		// Felhasználó adat (0x18)
		gdt[3] = gdt[2];
		// gdt[3].avl = 1;
		// gdt[3].limit1 = 0xffff;
		// gdt[3].limit2 = 0xf;
		gdt[3].access_byte.seg_desc.dpl = 3;

		// Felhasználó kód (0x20)
		gdt[4] = gdt[1];
		// gdt[4].avl = 1;
		// gdt[4].limit1 = 0xffff;
		// gdt[4].limit2 = 0xf;
		gdt[4].access_byte.seg_desc.dpl = 3;

		gdtr g = {
			sizeof(gdt_entry) * 8 - 1,
			(u64) gdt,
		};
		gdt_load(&g);
	}

	void add_tss(arch::tss::tss* t) {
		gdt[5].base1 = (u64)t;
		gdt[5].base2 = (u64)t >> 16;
		gdt[5].base3 = (u64)t >> 24;
		*(u32*)&gdt[6] = (u64)t >> 32;

		*(u8*)&(gdt[5].access_byte) = 0b10001001;
		gdt[5].access_byte.sys_seg_desc.dpl = 0;
		gdt[5].avl = 0;
		gdt[5].long_mode = 1;
		gdt[5].size = 0;
		gdt[5].limit_in_pages = 0;

		u32 limit = 104 - 1; // bitmap nélkül, ezért 104

		gdt[5].limit1 = limit & 0xf;
		gdt[5].limit2 = limit >> 4;
	}
}
