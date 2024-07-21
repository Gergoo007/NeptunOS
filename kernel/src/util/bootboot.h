#pragma once

#include <util/types.h>
#include <util/attrs.h>

#define mmap_ptr(a)  ((a)->ptr)
#define mmap_size(a) ((a)->size & 0xFFFFFFFFFFFFFFF0)
#define mmap_type(a) ((a)->size & 0xF)
#define mmap_free(a) (((a)->size&0xF)==1)

typedef struct _attr_packed {
	/* first 64 bytes is platform independent */
	u32 magic;		// "BOOT"
	u32 size;		// length of bootboot structure, minimum 128
	u8 protocol;	// 1, static addresses, see PROTOCOL_* and LOADER_* above
	u8 fb_type;		// framebuffer type, see FB_* above
	u16 numcores;	// number of processor cores
	u16 bspid;		// Bootstrap processor ID (Local APIC Id on x86_64)
	i16 timezone;	// in minutes -1440..1440
	u8 datetime[8];	// in BCD yyyymmddhhiiss UTC (independent to timezone)
	u64 initrd_ptr;	// ramdisk image position and size
	u64 initrd_size;
	u64 fb_ptr;		// framebuffer pointer and dimensions
	u32 fb_size;
	u32 fb_width;
	u32 fb_height;
	u32 fb_scanline;

	// the rest (64 bytes) is platform specific
	union {
		struct {
			u64 acpi_ptr;
			u64 smbi_ptr;
			u64 efi_ptr;
			u64 mp_ptr;
			u64 unused0;
			u64 unused1;
			u64 unused2;
			u64 unused3;
		} x86_64;
		struct {
			u64 acpi_ptr;
			u64 mmio_ptr;
			u64 efi_ptr;
			u64 unused0;
			u64 unused1;
			u64 unused2;
			u64 unused3;
			u64 unused4;
		} aarch64;
	} arch;

	// Memory map
	struct {
		u64 ptr;
		union {
			u64 size;
			u8 free : 1;
		};
	} mmap_entries[0];
} BOOTBOOT;

extern BOOTBOOT bootboot;
extern u32 fb;
extern u8 mmio;
extern char environment;
