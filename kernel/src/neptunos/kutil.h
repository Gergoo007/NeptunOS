#pragma once

#include <neptunos/efi/boot_env.h>

#include <neptunos/globals.h>
#include <neptunos/graphics/graphics.h>

#include <neptunos/graphics/text_renderer.h>
#include <neptunos/serial/serial.h>
#include <neptunos/libk/string.h>

#include <neptunos/memory/memory.h>
#include <neptunos/memory/efi_memory.h>
#include <neptunos/memory/paging.h>

#include <neptunos/tables/gdt.h>
#include <neptunos/tables/idt.h>
#include <neptunos/tables/interrupts.h>

#include <neptunos/power/power.h>

extern idtr idt;

void kinit(system_info* _info);
void int_prep();
void setup_paging();

extern volatile unsigned char _binary_font_psf_start;

