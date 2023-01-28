#pragma once

#include <neptunos/config/config.h>

#include <neptunos/efi/boot_env.h>
#include <neptunos/memory/memory.h>

extern uint16_t cursor_x;
extern uint16_t cursor_y;

extern system_info_t* info;

extern bitmap_t* bm;
extern void* mem_base;

extern void* KSTART;
extern void* KEND;

extern void __stack_chk_fail();
