#pragma once

#include <neptunos/globals.h>
#include <neptunos/graphics/text_renderer.h>

typedef struct rsdp {
	char signature[8];
	u8 checksum;
	char oem_id[6];
	u8 revision;
	u32 rsdt_addr;
} _attr_packed rsdp_t;

typedef struct xsdp {
	rsdp_t common;
	u32 length;
	u64 xsdt_addr;
	u8 checksum;
	u8 reserved[3];
} _attr_packed xsdp_t;

typedef struct sdt_hdr {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	uint8_t oem_id[6];
	uint8_t oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} _attr_packed sdt_hdr_t;

// PCIe related structs
typedef struct mcfg_entry {
	uint64_t base_addr;
	uint16_t segment_group_num;
	uint8_t bus_start;
	uint8_t bus_end;
	uint32_t reserved;
} _attr_packed mcfg_entry_t;

typedef struct mcfg_hdr_t {
	sdt_hdr_t hdr;
	uint64_t reserved;
} _attr_packed mcfg_hdr_t;

typedef struct madt_entry_base {
	u8 type;
	u8 length;
} madt_entry_base_t;

// APIC related structs

enum {
	MADT_ENTRY_LAPIC = 0,
	MADT_ENTRY_IOAPIC = 1,
	MADT_ENTRY_IOAPIC_OVERR = 2,
	MADT_ENTRY_IOAPIC_NMI_SRC = 3,
	MADT_ENTRY_LAPIC_NMI = 4,
	MADT_ENTRY_LAPIC_ADDR_OVERR = 5,
	MADT_ENTRY_LOCAL_X2APIC = 9,
};

typedef struct madt_hdr {
	sdt_hdr_t hdr;
	u32 lapic_addr;
	u32 flags;
	madt_entry_base_t entries[1];
} _attr_packed madt_hdr_t;

typedef struct madt_entry_flags {
	u8 active_low : 2;
	u8 lvl_trig : 2;
	u16 : 12;
} _attr_packed madt_entry_flags_t;

typedef struct madt_entry_lapic {
	madt_entry_base_t base;
	u8 cpu_id;
	u8 apic_id;
	u8 cpu_enabled : 1;
	u8 online_capable : 1;
	u32 : 30;
} _attr_packed madt_entry_lapic_t;

typedef struct madt_entry_int_override {
	madt_entry_base_t base;
	u8 bus_source;
	u8 irq_source;
	u32 gsi;
	madt_entry_flags_t flags;
} _attr_packed madt_entry_int_override_t;

typedef struct madt_entry_ioapic {
	madt_entry_base_t base;
	u8 ioapic_id;
	u8 res;
	u32 ioapic_addr;
	u32 gsi_base;
	madt_entry_flags_t flags;
} _attr_packed madt_entry_ioapic_t;

typedef struct madt_entry_nmi_src {
	madt_entry_base_t base;
	madt_entry_flags_t flags;
	u32 glob_sys_int;
} _attr_packed madt_entry_nmi_src;

typedef struct madt_entry_lapic_nmi {
	madt_entry_base_t base;
	u8 acpi_cpu_uid;
	madt_entry_flags_t flags;
	u8 lapic_lint;
} _attr_packed madt_entry_lapic_nmi_t;

typedef struct madt_entry_lapic_addr_override {
	madt_entry_base_t base;
	u16 res;
	u64 new_base;
} _attr_packed madt_entry_lapic_addr_override_t;

extern sdt_hdr_t* xsdt;
extern xsdp_t* xsdp;
extern mcfg_hdr_t* mcfg;
extern madt_hdr_t* madt;

void init_acpi(void);

void* get_table_address(char* table);
