#pragma once

#include <neptunos/globals.h>
#include <neptunos/graphics/text_renderer.h>

typedef struct rsdp_t {
	char signature[8];
	u8 checksum;
	char oem_id[6];
	u8 revision;
	u32 rsdt_addr;
} _attr_packed rsdp_t;

typedef struct xsdp_t {
	rsdp_t common;
	u32 length;
	u64 xsdt_addr;
	u8 checksum;
	u8 reserved[3];
} _attr_packed xsdp_t;

typedef struct sdt_hdr_t {
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

typedef struct mcfg_entry_t {
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

extern sdt_hdr_t* xsdt;
extern xsdp_t* xsdp;
extern mcfg_hdr_t* mcfg;

void init_acpi(void);

void* get_table_address(char* table);
