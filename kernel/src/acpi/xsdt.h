#pragma once

#include <lib/int.h>

#pragma pack(1)

typedef struct acpi_hdr_base {
	
} acpi_hdr_base_t;

#if rsdp_STRUCTS_DEFINED

#define rsdp_STRUCTS_DEFINED

typedef struct rsdp {
	char signature[8];
	u8 checksum;
	u8 oem_id[6];
	u8 rev;
	u32 rsdp_addr;
	u32 length;
	u64 xsdt_addr;
	u8 checksum_ext;
} rsdp_t;

typedef struct xsdt {
	char signature[4];
	u32 size;
	u8 rev;
	u8 checksum;
	u8 oem_id[6];
	u64 oem_table_id;
	u32 oem_rev;
	u32 creator_id;
	u32 creator_rev;
} xsdt_t;
#endif