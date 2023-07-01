#pragma once

#include <neptunos/config/config.h>

#include <neptunos/efi/boot_env.h>
#include <neptunos/memory/memory.h>

typedef struct {
	void* fb_base;
	u32 width;
	u32 height;
	u32 pitch;
	u32 bpp;
} _attr_packed screen_props_t;

extern uint16_t cursor_x;
extern uint16_t cursor_y;
extern boot_info_t* info;
extern screen_props_t screen;

extern void* KSTART;
extern void* KEND;
// extern void* BSS_START;
// extern void* BSS_END;

extern void __stack_chk_fail(void);
