#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct {
	uint32_t				type;
	uint32_t				pad;
	void*					phys_start;
	void*					virt_start;
	uint64_t				num_pages;
	uint64_t				attribute;
} __attribute__((packed)) efi_memory_descriptor_t;

typedef enum {
  	PixelRedGreenBlueReserved8BitPerColor,
  	PixelBlueGreenRedReserved8BitPerColor,
  	PixelBitMask,
  	PixelBltOnly,
  	PixelFormatMax
} __attribute__((packed)) efi_gop_pixel_format_t;

typedef struct {
	uint32_t				red_mask;
	uint32_t				green_mask;
	uint32_t				blue_mask;
	uint32_t				reserved_mask;
} __attribute__((packed)) efi_gop_pixel_bitmask_t;

typedef struct {
	uint32_t				version;
	uint32_t				width;
	uint32_t				height;
	efi_gop_pixel_format_t	pixel_format;
	efi_gop_pixel_bitmask_t	pixel_info;
	uint32_t				scanline;
} __attribute__((packed)) efi_gop_mode_info_t;

typedef struct {
	uint32_t				max_mode;
	uint32_t				mode;
	efi_gop_mode_info_t		*info;
	size_t					sizeof_info;
	void*					fb_base;
	size_t					fb_size;
} __attribute__((packed)) efi_gop_mode_t;

typedef struct memory_info {
	efi_memory_descriptor_t* mmap;
	size_t mmap_size;
	size_t mmap_key;
	size_t desc_size;
} __attribute__((packed)) memory_info;

typedef struct system_info {
	memory_info* mem_info;
	efi_gop_mode_t* g_info;
} __attribute__((packed)) system_info;
