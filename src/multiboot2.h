#pragma once

#define tag_align __attribute__((aligned(8)))

typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

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

typedef struct multiboot_fb_info_tag_t {
	uint32_t type;
	uint32_t size;
} __attribute__((packed)) multiboot_fb_info_tag_t;

typedef struct multiboot_tag_t {
	uint32_t type;
	uint32_t size;
} __attribute__((packed)) multiboot_tag_t;

struct multiboot_tag_framebuffer_common {
	uint32_t type;
	uint32_t size;

	uint64_t framebuffer_addr;
	uint32_t framebuffer_pitch;
	uint32_t framebuffer_width;
	uint32_t framebuffer_height;
	uint8_t framebuffer_bpp;
	#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
	#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
	#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT     2
	uint8_t framebuffer_type;
	uint16_t reserved;
};

struct multiboot_color {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

typedef struct multiboot_tag_framebuffer {
	struct multiboot_tag_framebuffer_common common;
	union {
		struct {
			uint16_t framebuffer_palette_num_colors;
			struct multiboot_color framebuffer_palette[0];
		};
		struct {
			uint8_t framebuffer_red_field_position;
			uint8_t framebuffer_red_mask_size;
			uint8_t framebuffer_green_field_position;
			uint8_t framebuffer_green_mask_size;
			uint8_t framebuffer_blue_field_position;
			uint8_t framebuffer_blue_mask_size;
		};
	};
} multiboot_tag_framebuffer_t;

typedef struct multiboot_hdr_t {
	uint32_t magic;
	uint32_t arch;
	uint32_t hdr_length;
	uint32_t checksum;

	struct {
		uint16_t type;
		uint16_t flags;
		uint32_t size;
		uint32_t* tag_types;
	} info_request;

	// struct {
	// 	uint32_t type;
	// 	uint32_t size;
	// 	uint32_t framebuffer_addr;
	// 	uint32_t framebuffer_pitch;
	// 	uint32_t framebuffer_width;
	// 	uint32_t framebuffer_height;
	// 	uint8_t framebuffer_bpp;
	// 	uint8_t framebuffer_type;
	// 	uint8_t reserved;
	// 	union {
	// 		struct {
	// 			uint32_t framebuffer_palette_num_colors;
	// 			struct {
	// 				uint8_t red_value;
	// 				uint8_t green_value;
	// 				uint8_t blue_value;
	// 			} framebuffer_palette;
	// 		} type_0;
	// 		struct {
	// 			uint8_t framebuffer_red_field_position;
	// 			uint8_t framebuffer_red_mask_size;
	// 			uint8_t framebuffer_green_field_position;
	// 			uint8_t framebuffer_green_mask_size;
	// 			uint8_t framebuffer_blue_field_position;
	// 			uint8_t framebuffer_blue_mask_size;
	// 		} type_1;
	// 	} color_info;
	// } tag_align fb_info_tag;

	struct {
		uint32_t type;
		uint32_t size;
		uint64_t pointer;
	} tag_align st_pointer;

	struct {
		uint32_t type;
		uint32_t size;
		char* str;
	} tag_align cmd_line;

	// Ending tag
	struct {
		uint16_t one;
		uint16_t two;
		uint32_t three;
	} tag_align end_tag;
} __attribute__((packed)) multiboot_hdr_t;
