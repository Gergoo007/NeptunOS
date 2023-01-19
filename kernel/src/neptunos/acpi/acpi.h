#pragma once

#include <neptunos/globals.h>
#include <neptunos/graphics/text_renderer.h>

typedef struct sdt_hdr_t {
	unsigned char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	uint8_t oem_id[6];
	uint8_t oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} _attr_packed sdt_hdr_t;

typedef struct mcfg_hdr_t {
	sdt_hdr_t hdr;
	uint64_t reserved;
} _attr_packed mcfg_hdr_t;

typedef struct device_config_t {
	uint64_t address;
	uint16_t pci_seg_group;
	uint8_t start_bus;
	uint8_t end_bus;
	uint32_t reserved;
} _attr_packed device_config_t;

void init_acpi();
