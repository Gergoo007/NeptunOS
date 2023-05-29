#pragma once

#include "stdint.h"

extern void* fb_base;
extern void* PSTART;
extern void* PEND;

static const char* EFI_MEMORY_TYPE_STRINGS[] = {
	"EfiReservedMemoryType",
	"EfiLoaderCode",
	"EfiLoaderData",
	"EfiBootServicesCode",
	"EfiBootServicesData",
	"EfiRuntimeServicesCode",
	"EfiRuntimeServicesData",
	"EfiConventionalMemory",
	"EfiUnusableMemory",
	"EfiACPIReclaimMemory",
	"EfiACPIMemoryNVS",
	"EfiMemoryMappedIO",
	"EfiMemoryMappedIOPortSpace",
	"EfiPalCode",
	"EfiPersistentMemory",
};

#define tag_align __attribute__((aligned(8)))

#define MULTIBOOT_TAG_TYPE_END               0
#define MULTIBOOT_TAG_TYPE_CMDLINE           1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME  2
#define MULTIBOOT_TAG_TYPE_MODULE            3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO     4
#define MULTIBOOT_TAG_TYPE_BOOTDEV           5
#define MULTIBOOT_TAG_TYPE_MMAP              6
#define MULTIBOOT_TAG_TYPE_VBE               7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER       8
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS      9
#define MULTIBOOT_TAG_TYPE_APM               10
#define MULTIBOOT_TAG_TYPE_EFI32             11
#define MULTIBOOT_TAG_TYPE_EFI64             12
#define MULTIBOOT_TAG_TYPE_SMBIOS            13
#define MULTIBOOT_TAG_TYPE_ACPI_OLD          14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW          15
#define MULTIBOOT_TAG_TYPE_NETWORK           16
#define MULTIBOOT_TAG_TYPE_EFI_MMAP          17
#define MULTIBOOT_TAG_TYPE_EFI_BS            18
#define MULTIBOOT_TAG_TYPE_EFI32_IH          19
#define MULTIBOOT_TAG_TYPE_EFI64_IH          20
#define MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR    21

#define MULTIBOOT_HEADER_TAG_END  0
#define MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST  1
#define MULTIBOOT_HEADER_TAG_ADDRESS  2
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS  3
#define MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS  4
#define MULTIBOOT_HEADER_TAG_FRAMEBUFFER  5
#define MULTIBOOT_HEADER_TAG_MODULE_ALIGN  6
#define MULTIBOOT_HEADER_TAG_EFI_BS        7
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI32  8
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI64  9
#define MULTIBOOT_HEADER_TAG_RELOCATABLE  10

#define MULTIBOOT_MEMORY_AVAILABLE			1
#define MULTIBOOT_MEMORY_RESERVED			2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE	3
#define MULTIBOOT_MEMORY_NVS				4
#define MULTIBOOT_MEMORY_BADRAM				5

typedef enum {
	EfiReservedMemoryType,
	EfiLoaderCode,
	EfiLoaderData,
	EfiBootServicesCode,
	EfiBootServicesData,
	EfiRuntimeServicesCode,
	EfiRuntimeServicesData,
	EfiConventionalMemory,
	EfiUnusableMemory,
	EfiACPIReclaimMemory,
	EfiACPIMemoryNVS,
	EfiMemoryMappedIO,
	EfiMemoryMappedIOPortSpace,
	EfiPalCode,
	EfiPersistentMemory,
	EfiMaxMemoryType
} EFI_MEMORY_TYPE;

typedef struct multiboot_tag_t {
	u32 type;
	u32 size;
} __attribute__((packed)) multiboot_tag_t;

struct multiboot_tag_framebuffer_common {
	u32 type;
	u32 size;

	u64 framebuffer_addr;
	u32 framebuffer_pitch;
	u32 framebuffer_width;
	u32 framebuffer_height;
	u8 framebuffer_bpp;
	#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
	#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
	#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT     2
	u8 framebuffer_type;
	u16 reserved;
};

struct multiboot_color {
	u8 red;
	u8 green;
	u8 blue;
};

typedef struct multiboot_tag_framebuffer {
	struct multiboot_tag_framebuffer_common common;
	union {
		struct {
			u16 framebuffer_palette_num_colors;
			struct multiboot_color framebuffer_palette[0];
		};
		struct {
			u8 framebuffer_red_field_position;
			u8 framebuffer_red_mask_size;
			u8 framebuffer_green_field_position;
			u8 framebuffer_green_mask_size;
			u8 framebuffer_blue_field_position;
			u8 framebuffer_blue_mask_size;
		};
	};
} multiboot_tag_framebuffer_t;

typedef struct multiboot_mmap_entry {
	u64 base_addr;
	u64 length;
	u32 type;
	u32 reserved;
} multiboot_mmap_entry_t;

typedef struct multiboot_tag_mmap {
	u32 type;
	u32 size;
	u32 entry_size;
	u32 entry_version;
	multiboot_mmap_entry_t first[1];
} multiboot_tag_mmap_t;

typedef struct multiboot_hdr_t {
	u32 magic;
	u32 arch;
	u32 hdr_length;
	u32 checksum;

	struct {
		u16 type;
		u16 flags;
		u32 size;
		u32 width;
		u32 height;
		u32 depth;
	} fb_tag;

	struct {
		u16 one;
		u16 two;
		u32 three;
	} end_tag;
} __attribute__((packed)) multiboot_hdr_t;

typedef struct {
	u32 type;
	u32 pad;
	u64 phys;
	u64 virt;
	u64 num_pages;
	u64 attr;
} efi_mem_desc_t;

typedef struct {
	u32 type;
	u32 size;
	u32 descr_size;
	u32 descr_vers;
	efi_mem_desc_t efi_mmap[];
} multiboot_tag_efi_mmap_t;

extern multiboot_hdr_t* mbi;
extern u64 mb_hdr_length;
