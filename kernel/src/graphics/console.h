#pragma once

#include <lib/types.h>
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

extern u16 con_cx, con_cy;
extern u32 con_bg, con_fg;

extern char* con_buffer;

typedef struct psf2_hdr {
	u32 magic;
	u32 version;
	u32 hdr_len;
	u32 flags;
	u32 num_glyphs;
	u32 glyphsize; // Byte-okban
	u32 height;
	u32 width;
} psf2_hdr;

typedef struct font {
	u64 hdr_ptr;
	u64 glyphs_ptr;
	u64 unicode_ptr;
	u32 glyphsize;
	u32 num_glyphs;
	u8 width;
	u8 height;
} font;

extern font con_font;

void console_clear();
void con_init(u32 fg, u32 bg);
void printk(const char* fmt, ...);
void con_draw();
void con_append(char* after);

void _report(const char* fmt, const char* file, ...);
void _warn(const char* fmt, const char* file, ...);
void _error(const char* fmt, const char* file, ...);

#define report(f, ...) _report(f, __FILE_NAME__, ##__VA_ARGS__)
#define warn(f, ...) _warn(f, __FILE_NAME__, ##__VA_ARGS__)
#define error(f, ...) _error(f, __FILE_NAME__, ##__VA_ARGS__)
