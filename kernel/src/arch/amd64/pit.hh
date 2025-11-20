#pragma once

#include <types.hh>

constexpr u32 PIT_IO_CH0 = 0x40;
constexpr u32 PIT_IO_CMD = 0x43;

punion pit_cmd_t {
	pstruct {
		u8 bcd : 1;
		u8 operation : 3;
		u8 access : 2;
		u8 ch : 2;
	};
	u8 raw;
};

enum {
	PIT_INT_ON_COUNT	= 0b000,
	PIT_HW_ONESHOT		= 0b001,
	PIT_RATE_GEN		= 0b010,
	PIT_SQUARE_WAVE		= 0b011,
	PIT_SW_STROBE		= 0b100,
	PIT_HW_STROBE		= 0b101,
};

enum {
	PIT_LATCH_COUNT_CMD	= 0b00,
	PIT_LOBYTE			= 0b01,
	PIT_HIBYTE			= 0b10,
	PIT_LOHIBYTE		= 0b11,
};

void arch_pit_init();
