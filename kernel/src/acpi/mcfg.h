#pragma once

#include <acpi/acpi.h>

#include <pci/pci.h>
#include <pci/strings.h>

#pragma pack(1)

typedef struct mcfg_entry {
	u64 base_address;
	u16 segment_group;
	u8 pci_bus_start;
	u8 pci_bus_end;
	u32 : 32;
} mcfg_entry_t;

typedef struct mcfg {
	sdt_base_t base;
	u64 : 64;
	mcfg_entry_t entries[0];
} mcfg_t;

void mcfg_parse(mcfg_t* mcfg);
