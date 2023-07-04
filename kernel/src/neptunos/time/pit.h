#pragma once

#include <neptunos/libk/stdall.h>
#include <neptunos/serial/com.h>

#define PIT_CH0 0x40
#define PIT_CMD 0x43

enum pit_op_modes {
	PIT_MODE_INT_ON_ZERO = 0b000,
	PIT_MODE_HW_TRIG_ONESHOT = 0b001,
	PIT_MODE_RATE_GEN = 0b010,
	PIT_MODE_SQW = 0b011,
	PIT_MODE_SW_TRIG_STROBE = 0b100,
	PIT_MODE_HW_TRIG_STROBE = 0b101,
	PIT_MODE_RATE_GEN_ = 0b110,
	PIT_MODE_SQW_ = 0b111,
};

enum pit_access_modes {
	PIT_ACCESS_LATCH_COUNT = 0b00,
	PIT_ACCESS_LOW_BYTE = 0b01,
	PIT_ACCESS_HIGH_BYTE = 0b10,
	PIT_ACCESS_LOHI_BYTE = 0b11,
};

typedef struct pit_mode {
	union {
		struct {
			u8 bcd : 1;
			u8 op_mode : 3;
			u8 access_mode : 2;
			u8 ch : 2;
		};
		u8 val;
	};
} _attr_packed pit_mode_t;

void pit_init(void);
