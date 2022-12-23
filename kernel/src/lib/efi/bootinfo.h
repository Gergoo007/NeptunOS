#pragma once

#include <stdint.h>
#include <stddef.h>

extern const char* EFI_MEMORY_TYPES[14];

typedef struct {
	void* BaseAddress;
	size_t BufferSize;
	unsigned int Width;
	unsigned int Height;
	unsigned int PixelsPerScanLine;
} framebuffer;

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

typedef struct {
	unsigned char magic[2];
	unsigned char mode;
	unsigned char charsize;
} psf1_header;

typedef struct {
	psf1_header* psf1_Header;
	void* glyphBuffer;
} psf1_font;

typedef struct {
	framebuffer* framebuffer;
	psf1_font* psf1_Font;
	void* mMap;
	uint64_t mMapSize;
	uint64_t mMapDescSize;
} BootInfo;

typedef struct EFI_MEMORY_DESCRIPTOR {
    uint32_t type;
    void* physAddr;
    void* virtAddr; 
    uint64_t numPages;
    uint64_t attribs;
} EFI_MEMORY_DESCRIPTOR;