#include <acpi/acpi.hh>
#include <arch/limine.hh>
#include <util/storage.hh>
#include <arch/arch.hh>
#include <arch/amd64/paging.hh>

__attribute__((section(".limine_requests"), used))
static volatile limine_rsdp_request rsdp_req {
	.id = LIMINE_RSDP_REQUEST,
	.revision = 0,
	.response = nullptr
};

namespace acpi {
	Vector<SDT*> tables;
	u8 ver;

	bool validate(SDT* table) {
		u8 sum = 0;
		for (u32 i = 0; i < table->length; i++)
			sum += ((u8*)table)[i];
		return sum ? false : true;
	}

	// Visszaadja az ACPI verziót, 0 ha érvénytelen
	u8 validateRsdp(RootPointer* table) {
		if (memcmp((void*)"RSD PTR ", table->sign, 8))
			return 0;

		// ACPI 2.0-s RSDP-e?
		u8 sum = 0;
		for (u32 i = 0; i < table->length; i++)
			sum += ((u8*)table)[i];

		if (!sum && table->xsdt)
			return 2;

		sum = 0;
		for (u32 i = 0; i < 20; i++)
			sum += ((u8*)table)[i];

		if (sum)
			return 0;
		else
			return 1;
	}

	static void check(void* a) {
		if (arch::paging_lookup((u64)a) == (u64)-1)
			arch::map_page(VIRTUAL((u64)a), (u64)PHYSICAL(a), (u32)arch::MFLAGS::KDATA);
	}

	void init() {
		auto r = rsdp_req.response;
		// Buggos ez az Istenverte szar
		check(r->address);

		if (!r || !r->address)
			error("Nincs RSDP??\n");

		acpi::RootPointer* rsdp = (acpi::RootPointer*)VIRTUAL(r->address);
		if (!(ver = validateRsdp(rsdp)))
			error("ACPI RSDP nem érvényes!\n");

		acpi::RootTable* rsdt = (acpi::RootTable*)VIRTUAL(ver == 2 ? rsdp->xsdt : rsdp->rsdt);
		check(rsdt);

		printk("RSDP ver: %d\n", ver);

		u32 num_tables;
		if (ver == 2)
			num_tables = (rsdt->hdr.length - sizeof(*rsdt)) / 8;
		else
			num_tables = (rsdt->hdr.length - sizeof(*rsdt)) / 4;

		for (u32 i = 0; i < num_tables; i++) {
			SDT* addr;
			if (ver == 2)
				addr = VIRTUAL((SDT*)rsdt->xsdtptrs[i]);
			else
				addr = VIRTUAL((SDT*)(u64)rsdt->rsdtptrs[i]);
			check(addr);
			tables.push_back(addr);
		}

		for (SDT* i : tables) {
			printk("%.4s: %p\n", (char*)&(i->sign), i);
		}
	}
}
