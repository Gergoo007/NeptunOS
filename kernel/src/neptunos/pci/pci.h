#pragma once

#include <neptunos/globals.h>

#include <neptunos/acpi/acpi.h>

#include <neptunos/memory/paging.h>

#include "pci_ids.h"

typedef struct pci_device_header_base_t {
	uint16_t vendor_id;
	uint16_t device_id;
	uint16_t command;
	uint16_t status;
	uint8_t rev_id;
	uint8_t program_interface;
	uint8_t subclass;
	uint8_t class;
	uint8_t cache_line_size;
	uint8_t latency_timer;
	uint8_t header_type;
	uint8_t bist;
} _attr_packed pci_device_header_base_t;

typedef struct pci_device_header_0_t {
	pci_device_header_base_t base;
	uint32_t base_addr_0;
	uint32_t base_addr_1;
	uint32_t base_addr_2;
	uint32_t base_addr_3;
	uint32_t base_addr_4;
	uint32_t base_addr_5;
	uint32_t cardbus_cis_pointer;
	uint16_t subsys_vendor_id;
	uint16_t subsys_id;
	uint32_t expansion_rom_base_addr;
	uint8_t capabilities_pointer;
	uint8_t reserved1[3];
	uint32_t reserved2;
	uint8_t int_line;
	uint8_t int_pin;
	uint8_t min_grant;
	uint8_t max_latency;
} pci_device_header_0_t;

typedef struct pci_device_header_1_t {
	pci_device_header_base_t base;
	uint32_t base_addr_0;
	uint32_t base_addr_1;
	uint8_t primary_bus_num;
	uint8_t secondary_bus_num;
	uint8_t subordinate_bus_num;
	uint8_t secondary_latency_timer;
	uint8_t io_base;
	uint8_t io_limit;
	uint16_t secondary_status;
	uint16_t mem_base;
	uint16_t mem_limit;
	uint32_t prefetch_base_upper;
	uint32_t prefetch_limit_upper;
	uint16_t io_base_upper;
	uint16_t io_limit_upper;
	uint8_t capability_pointer;
	uint8_t reserved[3];
	uint32_t expansion_rom_base_addr;
	uint8_t int_line;
	uint8_t int_pin;
	uint16_t bridge_ctl;
} pci_device_header_1_t;

typedef struct pci_device_header_t {
	pci_device_header_base_t base;
	union {
		pci_device_header_0_t type0;
		pci_device_header_1_t type1;
	};
} pci_device_header_t;

void pci_enumerate(void);

void pci_enumerate_bus(uint64_t base_addr, uint64_t bus);

void pci_enumerate_device(uint64_t bus_addr, uint64_t dev);

void pci_enumerate_function(uint64_t dev_addr, uint64_t func);
