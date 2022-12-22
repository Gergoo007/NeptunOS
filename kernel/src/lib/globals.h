#pragma once

#include "efi/bootinfo.h"

extern uint16_t cursor_x;
extern uint16_t cursor_y;

extern BootInfo* graphics;

extern uint64_t bitmap_size;

void init_globals();

void __stack_chk_fail();
