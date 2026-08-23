#include <arch/amd64/paging.hh>
#include <arch/amd64/apic.hh>
#include <arch/amd64/io.hh>
#include <arch/amd64/cpuid.hh>
#include <arch/amd64/smp.hh>
#include <arch/amd64/amd64.hh>

extern "C" { void x86_64_ap_trampoline(); void x86_64_ap_trampoline_end(); }

Vector<ioapic_t> ioapics;
u64 lapic_base = 0xfee00000ull;

Vector<lapic_t> cpus;

// Mind a 16 IRQ-hoz tartozhat egy
struct ioapic_redir {
	u32 gsi;
	bool activelow;
	bool lvltrig;
};

static Array<ioapic_redir, 16> redirection_table = {
	{ 0,  false, false },
	{ 1,  false, false },
	{ 2,  false, false },
	{ 3,  false, false },
	{ 4,  false, false },
	{ 5,  false, false },
	{ 6,  false, false },
	{ 7,  false, false },
	{ 8,  false, false },
	{ 9,  true,  true }, // SCI
	{ 10, false, false },
	{ 11, false, false },
	{ 12, false, false },
	{ 13, false, false },
	{ 14, false, false },
	{ 15, false, false }
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
				cpus.emplace_back(lapic_t {
					.apic_id = entry.MADT_LAPIC.apic_id,
					.acpi_id = entry.MADT_LAPIC.acpi_id,
					.up = false,
				});
				break;
			}
			case MadtTypes::MADT_IOAPIC: {
				map_page(VIRTUAL((u64)entry.MADT_IOAPIC.addr), (u64)entry.MADT_IOAPIC.addr, MFLAGS::KDATA, MCACHE::UC);
				ioapics.emplace_back(ioapic_t {
					.addr = (volatile u32*)VIRTUAL((u64)entry.MADT_IOAPIC.addr),
					.gsi_base = entry.MADT_IOAPIC.gsi_base
				});
				break;
			}
			case MadtTypes::MADT_OVERRIDE: {
				redirection_table[entry.MADT_OVERRIDE.irq].gsi = entry.MADT_OVERRIDE.gsi;
				redirection_table[entry.MADT_OVERRIDE.irq].activelow = entry.MADT_OVERRIDE.flags.active_low;
				redirection_table[entry.MADT_OVERRIDE.irq].lvltrig = entry.MADT_OVERRIDE.flags.lvl_triggered;
				// report("%d -> %d: %d %d", entry.MADT_OVERRIDE.irq, entry.MADT_OVERRIDE.gsi, entry.MADT_OVERRIDE.flags.lvl_triggered);
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
				ioapics.emplace_back(asd);
			} else if (maxent) {
				warn("Abnormalis IOAPIC GSI mennyiseg: %d", maxent);
				ioapics.emplace_back(asd);
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

	arch_ioapic_initialize_irq(0, 0x40, IoapicDelivmode::FIXED, bspid);
	arch_ioapic_mask_irq(0, 0);

	assert(cpuid_xapic_id() == 0);

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

	for (auto& i : ioapics)
		if (gsi >= i.gsi_base && gsi - i.gsi_base < 24) return i;
	fatal("No matching IOAPIC found for GSI #%d!", gsi);
}

void arch_ioapic_initialize_gsi(u32 gsi, u8 vector, IoapicDelivmode delivmode, u8 dest, bool activelow, bool lvl_trig) {
	ioapic_entry_t e {
			vector,
			delivmode,
			0,
			0,
			activelow,
			0,
			lvl_trig,
			1,
			dest
	};
	bestmatch(gsi).write(ioapic_register_t(gsi), e.qword);
}

void arch_ioapic_initialize_irq(u8 irq, u8 vector, IoapicDelivmode delivmode, u8 dest) {
	arch_ioapic_initialize_gsi(redirection_table[irq].gsi, vector, delivmode, dest, redirection_table[irq].activelow, redirection_table[irq].lvltrig);
}

void arch_ioapic_mask_gsi(u32 gsi, bool mask) {
	auto& t = bestmatch(gsi);
	ioapic_entry_t e;
	e.qword = t.read(ioapic_register_t(gsi));
	e.mask = mask;
	t.write(ioapic_register_t(gsi), e.qword);
}

void arch_ioapic_mask_irq(u8 irq, bool mask) {
	arch_ioapic_mask_gsi(redirection_table[irq].gsi, mask);
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
