#pragma once

#include <acpi/acpi.h>

#include <sysinfo.h>

#include <apic/apic.h>

#pragma pack(1)

enum {
	MADT_ENTRY_LAPIC = 0,
	MADT_ENTRY_IOAPIC = 1,
	MADT_ENTRY_OVERR = 2,
	MADT_ENTRY_IOAPIC_NMI = 3,
	MADT_ENTRY_LAPIC_NMI = 4,
	MADT_ENTRY_LAPIC_ADDR_OVERR = 5,
	MADT_ENTRY_X2APIC = 9,
};

extern u64 lapic_addr;

typedef struct madt {
	sdt_base_t base;
	u32 lapic_addr;
	u8 dual_pic : 1;
	u32 : 31;
} madt_t;

typedef struct madt_entry_base {
	u8 type;
	u8 size;
} madt_entry_base_t;

typedef struct madt_entry_lapic {
	madt_entry_base_t base;
	u8 acpi_cpu_id;
	u8 apic_id;
	// u32 flags;
	u8 cpu_enabled : 1;
	u8 online_cap : 1;
	u32 : 30;
} madt_entry_lapic_t;

typedef struct madt_entry_ioapic {
	madt_entry_base_t base;
	u8 id;
	u8 : 8;
	u32 addr;
	u32 gsi_base;
} madt_entry_ioapic_t;

typedef struct madt_entry_override {
	madt_entry_base_t base;
	u8 bus;
	u8 irq;
	u32 gsi;
	u16 flags;
} madt_entry_override_t;

typedef struct madt_entry_ioapic_nmi {
	madt_entry_base_t base;
	u8 nmi_source;
	u8 : 8;
	u16 flags;
	u32 gsi;
} madt_entry_ioapic_nmi_t;

typedef struct madt_entry_lapic_nmi {
	madt_entry_base_t base;
	u8 cpus;
	u16 flags;
	u8 lint : 1;
	u8 : 7;
} madt_entry_lapic_nmi_t;

typedef struct madt_entry_lapic_addr {
	madt_entry_base_t base;
	u16 : 16;
	u64 new_addr;
} madt_entry_lapic_addr_t;

typedef struct madt_entry_x2apic {
	madt_entry_base_t base;
	u16 : 16;
	u32 cpu_id;
	u32 flags;
	u32 acpi_id;
} madt_entry_x2apic_t;

void madt_parse(madt_t* madt);
