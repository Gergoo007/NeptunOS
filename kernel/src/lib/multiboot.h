#pragma once

#include <lib/int.h>
#include <lib/attrs.h>

#pragma pack(1)

enum MB_TAGS {
	MB_TAG_MEMMAP = 6,
	MB_TAG_FB = 8,
	MB_TAG_KERNEL_ADDR = 21,
	MB_TAG_rsdp = 14,
	MB_TAG_XSDP = 15,
};

typedef struct mb_tag_base {
	u32 type;
	u32 size;
} mb_tag_base_t;

typedef struct mb_tag_fb {
	mb_tag_base_t base;
	u64 fb_addr;
	u32 fb_pitch;
	u32 fb_width;
	u32 fb_height;
	u8 fb_bpp;
	u8 fb_type;
	u8 res;
	u8 color_info[0];
} mb_tag_fb_t;

typedef struct mb_memmap_entry {
	u64 addr;
	u64 length;
	u32 type;
	u32 _res;
} mb_memmap_entry_t;

typedef struct mb_tag_memmap {
	mb_tag_base_t base;
	u32 entry_size;
	u32 entry_ver;
	mb_memmap_entry_t entries[0];
} mb_tag_memmap_t;

typedef struct mb_tag_kaddr {
	mb_tag_base_t base;
	u32 addr;
} mb_tag_kaddr_t;

typedef struct sdt_base {
	char signature[4];
	u32 len;
	u8 rev;
	u8 checksum;
	char oem_id[6];
	u64 oem_table_id;
	u32 oem_rev;
	u32 creator_id;
	u32 creator_rev;
} sdt_base_t;

typedef struct rsdp {
	char signature[8];
	u8 checksum;
	u8 oem_id[6];
	u8 rev;
	u32 rsdt_addr;
	u32 length;
	u64 xsdt_addr;
	u8 checksum_ext;
} rsdp_t;

typedef struct rsdt {
	sdt_base_t base;
	u32 ptrs[0];
} rsdt_t;

typedef struct xsdt {
	sdt_base_t base;
	u64 ptrs[0];
} xsdt_t;

typedef struct mb_tag_rsdp {
	mb_tag_base_t base;
	union {
		xsdt_t xsdt;
		rsdp_t rsdp;
	};
} mb_tag_rsdp_t;
