#pragma once

#include <neptunos/config/attributes.h>
#include <neptunos/interrupts/idt.h>

// typedef struct interrupt_frame {
// 	u16 ip;
// 	u16 cs;
// 	u16 flags;
// 	u16 sp;
// 	u16 ss;
// } _attr_packed int_frame_t;

typedef struct interrupt_frame {
	u64 err_code;
	u64 rip;
	u64 cs;
	u64 flags;
	u64 rsp;
	u64 ss;
} _attr_packed int_frame_t;

extern u64 exception_isrs[][2];

_attr_int void ps2_kb_press(int_frame_t* frame);
_attr_int void nmi(int_frame_t* frame);
_attr_int void lint(int_frame_t* frame);
_attr_int void rtc_test(int_frame_t* frame);
_attr_int void pit_tick(int_frame_t* frame);
_attr_int void ps2_kb_press_pic(int_frame_t* frame);
