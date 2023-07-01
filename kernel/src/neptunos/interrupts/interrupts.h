#pragma once

#include <neptunos/config/attributes.h>
#include <neptunos/interrupts/idt.h>

typedef struct interrupt_frame {
	u16 ip;
	u16 cs;
	u16 flags;
	u16 sp;
	u16 ss;
} _attr_packed int_frame_t;

extern u64 exception_isrs[][2];

_attr_int void ps2_kb_press(int_frame_t* frame);
