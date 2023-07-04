#pragma once

#include <neptunos/efi/boot_env.h>

#include <neptunos/globals.h>
#include <neptunos/graphics/graphics.h>

#include <neptunos/graphics/text_renderer.h>
#include <neptunos/serial/serial.h>
#include <neptunos/libk/string.h>

#include <neptunos/memory/memory.h>
#include <neptunos/memory/paging.h>

#include <neptunos/gdt/gdt.h>
#include <neptunos/interrupts/idt.h>
#include <neptunos/interrupts/interrupts.h>

#include <neptunos/power/power.h>

#include <neptunos/acpi/acpi.h>

#include <neptunos/cpuid/cpuid.h>
#include <neptunos/interrupts/pic/pic.h>

extern uint16_t get_processor_id(void);

void kinit(void);

extern volatile unsigned char _binary_font_psf_start;
extern volatile unsigned char _binary_font_psf_end;

