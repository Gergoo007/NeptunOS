#pragma once

#include "neptunos/config/attributes.h"
#include <neptunos/libk/stdint.h>
#include <neptunos/libk/stddef.h>

typedef struct {
	uint32_t				type;
	uint32_t				pad;
	void*					phys_start;
	void*					virt_start;
	uint64_t				num_pages;
	uint64_t				attribute;
} _attr_packed efi_memory_descriptor_t;

typedef enum {
  	PixelRedGreenBlueReserved8BitPerColor,
  	PixelBlueGreenRedReserved8BitPerColor,
  	PixelBitMask,
  	PixelBltOnly,
  	PixelFormatMax
} _attr_packed efi_gop_pixel_format_t;

typedef struct {
	uint32_t				red_mask;
	uint32_t				green_mask;
	uint32_t				blue_mask;
	uint32_t				reserved_mask;
} _attr_packed efi_gop_pixel_bitmask_t;

typedef struct {
	uint32_t				version;
	uint32_t				width;
	uint32_t				height;
	efi_gop_pixel_format_t	pixel_format;
	efi_gop_pixel_bitmask_t	pixel_info;
	uint32_t				scanline;
} _attr_packed efi_gop_mode_info_t;

typedef struct {
	uint32_t				max_mode;
	uint32_t				mode;
	efi_gop_mode_info_t		*info;
	size_t					sizeof_info;
	void*					fb_base;
	size_t					fb_size;
} _attr_packed efi_gop_mode_t;

typedef struct rsdp_desc_t {
	char signature[8];
	uint8_t checksum;
	char oemid[6];
	uint8_t rev;
	uint32_t rsdt_address;
} __attribute__ ((packed)) rsdp_desc_t;

typedef struct rsdp_desc_2_t {
	rsdp_desc_t first;
	uint32_t length;
	uint64_t xsdt_address;
	uint8_t ext_checksum;
	uint8_t reserved[3];
} __attribute__ ((packed)) rsdp_desc_2_t;

typedef struct memory_info_t {
	efi_memory_descriptor_t* mmap;
	size_t mmap_size;
	size_t mmap_key;
	size_t desc_size;
	void* rsdp;
} _attr_packed memory_info_t;

typedef struct system_info_t {
	memory_info_t* mem_info;
	efi_gop_mode_t* g_info;
} _attr_packed system_info_t;
