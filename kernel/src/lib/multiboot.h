#pragma once

#include <lib/int.h>
#include <lib/attrs.h>

#pragma pack(1)

enum MB_TAGS {
	MB_TAG_MEMMAP = 6,
	MB_TAG_FB = 8,
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
