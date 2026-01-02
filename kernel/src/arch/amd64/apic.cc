#include <arch/amd64/paging.hh>
#include <arch/amd64/apic.hh>
#include <arch/amd64/io.hh>
#include <arch/amd64/cpuid.hh>
#include <arch/amd64/smp.hh>
#include <arch/amd64/amd64.hh>

extern "C" { void x86_64_ap_trampoline(); void x86_64_ap_trampoline_end(); }

vector<ioapic_t> ioapics;
u64 lapic_base = 0xfee00000;

vector<lapic_t> cpus;

// Mind a 16 IRQ-hoz tartozhat egy
static array<16, u32> redirection_table = {
	0, 1,  2,  3,  4,  5,  6,  7,
	8, 9, 10, 11, 12, 13, 14, 15
};

u32 lapic_read(const lapic_register_t& reg) {
	return *((volatile u32*)(lapic_base + reg.offset));
}

void lapic_write(const lapic_register_t& reg, u32 val) {
	*((volatile u32*)(lapic_base + reg.offset)) = val;
}

void arch_parse_madt(madt_t* m) {
	u32 offset = 0;
	while (sizeof(madt_t) + offset < m->sdt.length) {
		auto& entry = *(madt_entry_t*)((u64)m + sizeof(madt_t) + offset);

		switch (entry.type) {
			case MadtTypes::MADT_LAPIC: {
				// Van néhány buggos szar firmware ahol több LAPIC entry van mint logikai processzor, 0-ás APIC ID-val
				// Ezekkel nem kell foglalkozni
				if (entry.MADT_LAPIC.apic_id == 0)
					break;
				cpus.emplace(lapic_t {
					.apic_id = entry.MADT_LAPIC.apic_id,
					.acpi_id = entry.MADT_LAPIC.acpi_id,
					.up = false,
				});
				break;
			}
			case MadtTypes::MADT_IOAPIC: {
				map_page(VIRTUAL((u64)entry.MADT_IOAPIC.addr), (u64)entry.MADT_IOAPIC.addr, MFLAGS::KDATA, MCACHE::UC);
				ioapics.emplace(ioapic_t {
					.addr = (volatile u32*)VIRTUAL((u64)entry.MADT_IOAPIC.addr),
					.gsi_base = entry.MADT_IOAPIC.gsi_base
				});
				break;
			}
			case MadtTypes::MADT_OVERRIDE: {
				redirection_table[entry.MADT_OVERRIDE.irq] =
					entry.MADT_OVERRIDE.gsi;
				break;
			}
			case MadtTypes::MADT_IOAPIC_NMI: {

				break;
			}
			case MadtTypes::MADT_LAPIC_NMI: {

				break;
			}
			case MadtTypes::MADT_LAPIC_ADDR: {
				lapic_base = entry.MADT_LAPIC_ADDR.lapic;
				break;
			}
			case MadtTypes::MADT_LAPIC_X2APIC: {

				break;
			}
		}

		offset += entry.length;
	}

	// PIC kikapcs.
	outb(0x21, 0xff);
	outb(0xa1, 0xff);

	if constexpr (IOAPIC_FIX) {
		if (!ioapics.size) {
			map_page(VIRTUAL((u64)0xfec00000), 0xfec00000, MFLAGS::KDATA, MCACHE::UC);
			ioapic_t asd {
				.addr = (volatile u32*)VIRTUAL((u64)0xfec00000),
				.gsi_base = 0,
			};
			u32 ver = asd.read(IoapicRegs::VER);
			u8 maxent = (ver >> 16) & 0xff;
			// Normális mennyiség
			if (maxent == 23) {
				warn("ACPI szerint nincs IOAPIC, amugy meg van");
				ioapics.emplace(asd);
			} else if (maxent) {
				warn("Abnormalis IOAPIC GSI mennyiseg: %d", maxent);
				ioapics.emplace(asd);
			} else {
				// Nincs IOAPIC
				fatal("Nincs IOAPIC!");
			}
		}
	}

	// LAPIC bekapcs
	map_page(VIRTUAL(lapic_base), lapic_base, MFLAGS::KDATA, MCACHE::UC);
	lapic_base = VIRTUAL(lapic_base);

	lapic_write(LapicRegs::SIV, lapic_read(LapicRegs::SIV) | 0x100);

	u32 bspid = cpuid_xapic_id();
	for (auto& cpu : cpus) {
		if (cpu.apic_id == bspid) {
			cpu.up = true;
			break;
		}
	}

	arch_ioapic_initialize_irq(2, 0x40, IoapicDelivmode::FIXED, 1, bspid);
	arch_ioapic_mask_gsi(2, 0);

	report("x2apic id: %d", cpuid_x2apic_id());
	report("xapic  id: %d", cpuid_x2apic_id());

	smp_init();

	arch_start_timer();
	for (auto& cpu : cpus) {
		while (!cpu.up) {
			if (arch_elapsed(300)) {
				error("Timeout for CPU %d (ACPI ID %d)!", cpu.apic_id, cpu.acpi_id);
				break;
			}
		}
	}

	report("All CPUs up!");
}

// Visszaadja a GSI-hoz lévő legközelebbi IOAPIC-ot
static ioapic_t& bestmatch(u32 gsi) {
	assert(ioapics.size);

	ioapic_t& a = ioapics[0];
	u32 smallestdiff = gsi - ioapics[0].gsi_base;
	for (const auto& i : ioapics) {
		if (a.gsi_base < gsi && gsi - i.gsi_base) {
			a = i;
			smallestdiff = gsi - i.gsi_base;
		}
	}

	if (smallestdiff > 24) {
		error("Failed to find GSI's IOAPIC! Closest match's GSI base: %d; searchee: %d", a.gsi_base, gsi);
	}
	
	return a;
}

void arch_ioapic_assign_vector_gsi(u8 irq, u8 vec) {
	u32 gsi = redirection_table[irq];
	u64 entry = vec;
	bestmatch(gsi).write(ioapic_register_t(gsi), entry);
}

void arch_ioapic_initialize_gsi(u32 gsi, u8 vector, IoapicDelivmode delivmode, bool activelow, u8 dest) {
	ioapic_entry_t e {
			vector,
			delivmode,
			0,
			0,
			activelow,
			0,
			0,
			1,
			dest
	};
	bestmatch(gsi).write(ioapic_register_t(gsi), e.qword);
}

void arch_ioapic_initialize_irq(u8 irq, u8 vector, IoapicDelivmode delivmode, bool activelow, u8 dest) {
	arch_ioapic_initialize_gsi(redirection_table[irq], vector, delivmode, activelow, dest);
}

void arch_ioapic_mask_gsi(u32 gsi, bool mask) {
	auto& t = bestmatch(gsi);
	ioapic_entry_t e;
	e.qword = t.read(ioapic_register_t(gsi));
	e.mask = mask;
	t.write(ioapic_register_t(gsi), e.qword);
}

void arch_ioapic_mask_irq(u8 irq, bool mask) {
	arch_ioapic_mask_gsi(redirection_table[irq], mask);
}

void arch_lapic_eoi() {
	lapic_write(LapicRegs::EOI, 0x00);
}

void arch_ioapic_disable_all() {
	for (auto& ioa : ioapics) {
		ioapic_entry_t e;
		for (u32 i = 0; i < 24; i++) {
			e.qword = ioa.read(ioapic_register_t(i));
			e.mask = true;
			ioa.write(ioapic_register_t(i), e.qword);
		}
	}
}
