#pragma once

#include <lib/int.h>
#include <serial/serial.h>
#include <graphics/graphics.h>
#include <memory/heap/vmm.h>

#include <config.h>

#include <lib/attrs.h>

#pragma pack(1)

#define PSF2_IDENT 0x864ab572

#define FONT_START _binary_zap_ext_light24_psf_start
#define FONT_END _binary_zap_ext_light24_psf_end
#define FONT_SIZE _binary_zap_ext_light24_psf_size

extern u64 _binary_zap_ext_light24_psf_start;
extern u64 _binary_zap_ext_light24_psf_end;
extern u64 _binary_zap_ext_light24_psf_size;

extern u16 cx, cy;
extern u32 bg, fg;

typedef struct psf2_hdr {
	u32 magic;
	u32 version;
	u32 hdr_len;
	u32 flags;
	u32 num_glyphs;
	u32 glyphsize; // Byte-okban
	u32 height;
	u32 width;
} psf2_hdr_t;

typedef struct font {
	u64 hdr_ptr;
	u64 glyphs_ptr;
	u64 unicode_ptr;
	u32 glyphsize;
	u32 num_glyphs;
	u8 width;
	u8 height;
} font_t;

void con_init(u32 fg, u32 bg);
void kputc(char c);
void kputs(char* s);
void printk(const char* fmt, ...);
