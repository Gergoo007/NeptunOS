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

extern u64 int_handlers[][2];

_attr_int void page_flt_handler(struct interrupt_frame* frame);
_attr_int void double_flt_handler(struct interrupt_frame* frame);
_attr_int void invalid_opcode_flt_handler(struct interrupt_frame* frame);
_attr_int void general_protection_handler(struct interrupt_frame* frame);
_attr_int void custom_handler(struct interrupt_frame* frame);
_attr_int void pic_kb_press(struct interrupt_frame* frame);
_attr_int void pit_tick_int(struct interrupt_frame* frame);
