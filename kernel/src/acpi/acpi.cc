#include <acpi/acpi.hh>
#include <acpi/aml.hh>
#include <arch/limine.hh>
#include <util/storage.hh>
#include <arch/arch.hh>
#include <arch/amd64/paging.hh>
#include <arch/amd64/apic.hh>
#include <pci/pci.hh>

#include <arch/amd64/io.hh>

__attribute__((section(".limine_requests"), used))
static volatile struct limine_rsdp_request rsdp_req = {
	.id = LIMINE_RSDP_REQUEST,
	.revision = 0,
	.response = nullptr
};

vector<sdt_t*> tables;
u8 ver;

bool validate(sdt_t* table) {
	u8 sum = 0;
	for (u32 i = 0; i < table->length; i++)
		sum += ((u8*)table)[i];
	return sum ? false : true;
}

// Visszaadja az ACPI verziót, 0 ha érvénytelen
u8 validateRsdp(rootptr_t* table) {
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
	if (paging_lookup((u64)a) == (u64)-1)
		map_page(VIRTUAL((u64)a), PHYSICAL((u64)a), (u32)MFLAGS::KDATA);
}

static void checkTable(sdt_t* a) {
	check(a);
	for (u64 i = 0; i < align(a->length, 0x1000); i += 0x1000) {
		check((u8*)a + i);
	}
}

static void process_fadt(fadt_t* fadt) {
	fadt = VIRTUAL(fadt);
	checkTable((sdt_t*)fadt);

	sdt_t* dsdt = VIRTUAL((sdt_t*)fadt->x_dsdt);
	check(dsdt);
	if (dsdt->sign != 'TDSD')
		dsdt = (sdt_t*)(u64)fadt->dsdt;
	dsdt = VIRTUAL(dsdt);
	checkTable(dsdt);

	if (!validate(dsdt))
		error("Hibas DSDT!");

	// process_aml((OPCODES*)practice + sizeof(sdt_t), sizeof(practice));
}

void acpi_init() {
	auto r = rsdp_req.response;
	// Buggos ez az Istenverte szar
	check((void*)r->address);

	if (!r || !r->address)
		error("Nincs RSDP??");

	rootptr_t* rsdp = (rootptr_t*)VIRTUAL(r->address);
	if (!(ver = validateRsdp(rsdp)))
		error("ACPI RSDP nem érvényes!");

	roottable_t* rsdt = (roottable_t*)VIRTUAL(ver == 2 ? rsdp->xsdt : rsdp->rsdt);
	check(rsdt);

	u32 num_tables;
	if (ver == 2)
		num_tables = (rsdt->hdr.length - sizeof(sdt_t)) / 8;
	else
		num_tables = (rsdt->hdr.length - sizeof(sdt_t)) / 4;

	for (u32 i = 0; i < num_tables; i++) {
		sdt_t* addr;
		if (ver == 1 || ver == 0)
			addr = VIRTUAL((sdt_t*)(u64)VIRTUAL(((u32*)&rsdt->arraystart)[i]));
		else
			addr = VIRTUAL((sdt_t*)(u64)VIRTUAL(((u64*)&rsdt->arraystart)[i]));
		check(addr);
		tables.emplace(addr);
	}

	for (sdt_t* i : tables) {
		switch (i->sign) {
			case AcpiSignatures::MCFG:
				mcfg = (mcfg_t*)i;
				break;
			case AcpiSignatures::APIC:
				arch_parse_madt((madt_t*)i);
				break;
		}
	}

	for (sdt_t* i : tables) {
		switch (i->sign) {
			case AcpiSignatures::FADT:
				auto* fadt = (fadt_t*)i;
				sdt_t* dsdt;
				// 4 GiB felett van az FACS? Ha igen, akkor az X_* mezőket kell hazsnálni
				if ((u64)PHYSICAL(fadt) >> 32) {
					dsdt = (sdt_t*)fadt->x_dsdt;
				} else {
					dsdt = (sdt_t*)(u64)fadt->dsdt;
				}
				// error("dsdt @ %p; enabling acpi", dsdt);
				// arch_ioapic_initialize_irq(fadt->sci_int, 0x42, IoapicDelivmode::FIXED, 0);
				// arch_ioapic_mask_irq(fadt->sci_int, 0);
				// error("%d %d %d", fadt->acpi_enable, fadt->acpi_disable, inb(fadt->smi_cmd_port));
				// error("turi %d", inw(fadt->pm1a_ctl_blk) & 1);
				// outb(fadt->smi_cmd_port, fadt->acpi_enable);
				// error("%d %d %d", fadt->acpi_enable, fadt->acpi_disable, inb(fadt->smi_cmd_port));
				
				// error("turi %d", inw(fadt->pm1a_ctl_blk) & 1);

				sprintk("\n\r");
				dsdt = VIRTUAL(dsdt);
				check_page((u64)dsdt, MCACHE::WB);
				check_pages((u64)dsdt + 0x1000, dsdt->length);
				acpi_parse_aml(dsdt);
				break;
		}
	}
}
