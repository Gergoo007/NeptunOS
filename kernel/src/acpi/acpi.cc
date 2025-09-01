#include <acpi/acpi.hh>
#include <arch/limine.hh>
#include <util/storage.hh>

__attribute__((section(".limine_requests"), used))
static volatile limine_rsdp_request rsdp_req {
	.id = LIMINE_RSDP_REQUEST,
	.revision = 0,
	.response = nullptr
};

namespace acpi {
	Vector<SDT*> tables;

	bool validate(SDT* table) {
		u8 sum = 0;
		for (u32 i = 0; i < table->length; i++)
			sum += ((u8*)table)[i];
		return sum ? false : true;
	}

	void init() {
		auto r = rsdp_req.response;
		if (!r)
			error("Nincs RSDP??\n");

		tables.push_back((acpi::SDT*)VIRTUAL(r->address));
	}
}
