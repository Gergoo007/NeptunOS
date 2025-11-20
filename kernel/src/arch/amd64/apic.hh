#pragma once

#include <types.hh>
#include <acpi/acpi.hh>
#include <util/storage.hh>

enum struct MadtTypes : u8 {
	MADT_LAPIC			= 0,
	MADT_IOAPIC			= 1,
	MADT_OVERRIDE		= 2,
	MADT_IOAPIC_NMI		= 3,
	MADT_LAPIC_NMI		= 4,
	MADT_LAPIC_ADDR		= 5,
	MADT_LAPIC_X2APIC	= 9,
};

pstruct madt_flags_t {
	u16 : 1;
	u16 active_low : 1;
	u16 : 1;
	u16 lvl_triggered : 1;
	u16 : 12;
};

pstruct madt_t {
	sdt_t sdt;
	pstruct {
		MadtTypes type;
		u8 len;
		punion {
			pstruct {
				u8 acpi_id;
				u8 apic_id;
				u32 enabled : 1;
				u32 capable : 1;
				u32 : 30;
			} MADT_LAPIC;
			pstruct {
				u8 id;
				u8 : 8;
				u32 addr;
				u32 gsi_base;
			} MADT_IOAPIC;
			pstruct {
				u8 bus;
				u8 irq;
				u32 gsi;
				madt_flags_t flags;
			} MADT_OVERRIDE;
			pstruct {
				u8 nmi_src;
				u8 : 8;
				madt_flags_t flags;
				u32 gsi;
			} MADT_IOAPIC_NMI;
			pstruct {
				u8 acpi_id; // 0xff: összes cpu
				madt_flags_t flags;
				u8 lint; // LINT0 vagy LINT1
			} MADT_LAPIC_NMI;
			pstruct {
				u16 : 16;
				u64 lapic;
			} MADT_LAPIC_ADDR;
			pstruct {
				u16 : 16;
				u32 x2apic_id;
				u32 enabled : 1;
				u32 capable : 1;
				u32 : 30;
				u32 acpi_id;
			} MADT_LAPIC_X2APIC;
		};
	} entries[0];
};

struct ioapic_register_t {
	u8 offset;
	u8 size; // 0: 32 bit, 1: 64 bit

	// IRQ
	constexpr ioapic_register_t(u8 irq): offset(0x10 + irq*2), size(1) {}
	constexpr ioapic_register_t(u8 o, u8 s): offset(o), size(s) {}
};

struct IoapicRegs {
	static constexpr ioapic_register_t ID		{ 0x00, 0 };
	static constexpr ioapic_register_t VER		{ 0x01, 0 };
	static constexpr ioapic_register_t VEARB	{ 0x02, 0 };
	static constexpr ioapic_register_t VEC0		{ 0X10, 1 };
};

struct ioapic_t {
	volatile u32* addr;
	u32 gsi_base;

	u64 read(const ioapic_register_t& reg) {
		*addr = (u32)reg.offset;
		u64 ret = *(addr + 4);
		if (reg.size) {
			*addr = (u32)reg.offset + 1;
			ret |= ((u64)*(addr + 4)) << 32;
		}
		return ret;
	}

	void write(const ioapic_register_t& reg, u64 val) {
		*addr = (u32)reg.offset;
		*(addr + 4) = val;
		if (reg.size) {
			*addr = (u32)reg.offset + 1;
			*(addr + 4) = (val >> 32) & 0xffffffff;
		}
	}
};

struct redirection_t {
	u32 gsi;
	u8 irq;
	madt_flags_t flags;
};

enum IoapicDelivmode : u64 {
	FIXED		= 0b000,
	LOWESTPRIO	= 0b001,
	SMI			= 0b010,
	NMI			= 0b100,
	INIT		= 0b101,
	EXTINT		= 0b111,
};

punion ioapic_entry_t {
	pstruct {
		u64 vector		: 8;
		u64 delivmode	: 3;
		u64 logicaldest	: 1; // 1: logical, 0: physical címzett
		u64 delivered	: 1; // 0: el lett küldve de nem kapta meg a címzett
		u64 activelow	: 1;
		u64 remoteirr	: 1;
		u64 lvl_triggered: 1;
		u64 mask		: 1;
		u64 			: 39;
		u64 destination	: 8;
	};
	u64 qword;
};

struct lapic_t {
	u8 apic_id;
	u8 acpi_id;
};

struct lapic_register_t { u32 offset; };
struct LapicRegs {
	static constexpr lapic_register_t EOI { 0x0b0 };
	static constexpr lapic_register_t SIV { 0x0f0 };
};

void arch_parse_madt(madt_t* m);
void arch_ioapic_initialize_gsi(u32 gsi, u8 vector, IoapicDelivmode delivmode, bool activelow, u8 dest);
void arch_ioapic_initialize_irq(u8 irq, u8 vector, IoapicDelivmode delivmode, bool activelow, u8 dest);
void arch_ioapic_mask_gsi(u32 gsi, bool mask);
void arch_ioapic_mask_irq(u8 irq, bool mask);

void arch_lapic_eoi();
