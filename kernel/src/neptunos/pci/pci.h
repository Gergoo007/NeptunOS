#pragma once

#include <neptunos/globals.h>

#include <neptunos/acpi/acpi.h>

#include <neptunos/memory/paging.h>

typedef struct pci_device_header_t {
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
} _attr_packed pci_device_header_t;

void pci_enumerate();

void pci_enumerate_bus(uint64_t base_addr, uint64_t bus);

void pci_enumerate_device(uint64_t bus_addr, uint64_t dev);

void pci_enumerate_function(uint64_t dev_addr, uint64_t func);
