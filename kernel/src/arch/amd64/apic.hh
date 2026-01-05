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

pstruct madt_entry_t {
	MadtTypes type;
	u8 length;

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
};
static_assert(offsetof(madt_entry_t, length) == 1);

pstruct madt_t {
	sdt_t sdt;
	u32 stuff;
	u32 stuff2;
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

	// If thgis is an AP, that AP will set this to true after bootstrap
	volatile bool up;
};

struct lapic_register_t { u32 offset; };
struct LapicRegs {
	static constexpr lapic_register_t ID	{ 0x020 };	//	LAPIC ID Register	Read/Write
	static constexpr lapic_register_t VER	{ 0x030 };	//	LAPIC Version Register	Read only
	static constexpr lapic_register_t TPR	{ 0x080 };	//	Task Priority Register (TPR)	Read/Write
	static constexpr lapic_register_t APR	{ 0x090 };	//	Arbitration Priority Register (APR)	Read only
	static constexpr lapic_register_t PPR	{ 0x0A0 };	//	Processor Priority Register (PPR)	Read only
	static constexpr lapic_register_t EOI	{ 0x0B0 };	//	EOI register	Write only
	static constexpr lapic_register_t RRD	{ 0x0C0 };	//	Remote Read Register (RRD)	Read only
	static constexpr lapic_register_t LDR	{ 0x0D0 };	//	Logical Destination Register	Read/Write
	static constexpr lapic_register_t DFR	{ 0x0E0 };	//	Destination Format Register	Read/Write
	static constexpr lapic_register_t SIV	{ 0x0F0 };	//	Spurious Interrupt Vector Register	Read/Write
	static constexpr lapic_register_t ISR	{ 0x100 };	// - 170h	In-Service Register (ISR)	Read only
	static constexpr lapic_register_t TMR	{ 0x180 };	// - 1F0h	Trigger Mode Register (TMR)	Read only
	static constexpr lapic_register_t IRR	{ 0x200 };	// - 270h	Interrupt Request Register (IRR)	Read only
	static constexpr lapic_register_t ERR	{ 0x280 };	//	Error Status Register	Read only
	static constexpr lapic_register_t ICR0	{ 0x300 };	// Interrupt Command Register (ICR)	Read/Write
	static constexpr lapic_register_t ICR1	{ 0x310 };	// Interrupt Command Register (ICR)	Read/Write
	
	static constexpr lapic_register_t LVT_CMCI		{ 0x2F0 };	//	LVT Corrected Machine Check Interrupt (CMCI) Register	Read/Write
	static constexpr lapic_register_t LVT_TIMER		{ 0x320 };	//	LVT Timer Register	Read/Write
	static constexpr lapic_register_t LVT_THERMAL	{ 0x330 };	//	LVT Thermal Sensor Register	Read/Write
	static constexpr lapic_register_t LVT_PERF		{ 0x340 };	//	LVT Performance Monitoring Counters Register	Read/Write
	static constexpr lapic_register_t LVT_LINT0		{ 0x350 };	//	LVT LINT0 Register	Read/Write
	static constexpr lapic_register_t LVT_LINT1		{ 0x360 };	//	LVT LINT1 Register	Read/Write
	static constexpr lapic_register_t LVT_ERROR		{ 0x370 };	//	LVT Error Register	Read/Write

	static constexpr lapic_register_t TMR_INITIAL_COUNT	{ 0x380 };	//	Initial Count Register (for Timer)	Read/Write
	static constexpr lapic_register_t TMR_COUNT			{ 0x390 };	//	Current Count Register (for Timer)	Read only
	static constexpr lapic_register_t TMR_DIVIDE		{ 0x3E0 };	//	Divide Configuration Register (for Timer)	Read/Write
};

extern vector<lapic_t> cpus;

void arch_parse_madt(madt_t* m);
void arch_ioapic_initialize_gsi(u32 gsi, u8 vector, IoapicDelivmode delivmode, u8 dest, bool activelow, bool lvl_trig = false);
void arch_ioapic_initialize_irq(u8 irq, u8 vector, IoapicDelivmode delivmode, u8 dest);
void arch_ioapic_mask_gsi(u32 gsi, bool mask);
void arch_ioapic_mask_irq(u8 irq, bool mask);
void arch_ioapic_disable_all();

void arch_lapic_eoi();
