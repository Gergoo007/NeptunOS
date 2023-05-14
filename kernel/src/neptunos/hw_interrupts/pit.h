#pragma once

#include <neptunos/graphics/text_renderer.h>

#define PIT_IO_CH_0 			0x40

// Shouldn't be used, as it isn't guaranteed to be implemented on more recent machines
#define PIT_IO_CH_1 			0x41

#define PIT_IO_CH_2 			0x42
#define PIT_IO_MODE_CMD 		0x43

#define PIT_CH_0				0b00;
#define PIT_CH_1				0b01;
#define PIT_CH_2				0b10;
#define PIT_CH_READ_BACK 		0b11;

#define PIT_LATCH_COUNT_VAL 	0b00
#define PIT_LOBYTE				0b01
#define PIT_HIBYTE				0b10
#define PIT_LOHIBYTE			0b11

#define PIT_INT_ON_TERM_COUNT 	0b000
#define PIT_HW_ONESHOT 			0b001
#define PIT_RATE_GEN			0b010
#define PIT_SQUARE_WAVE_GEN		0b011
#define PIT_SW_TRIG_STROBE		0b100
#define PIT_HW_TRIG_STROBE		0b101

typedef struct pit_command_t {
	uint8_t bcd_mode 	: 1;
	uint8_t oper_mode	: 3;
	uint8_t access_mode	: 2;
	uint8_t channel		: 2;
} _attr_packed pit_command_t;

void pit_configure_channel(uint8_t ch);

void pit_delay(uint64_t millis);

void pit_set_divisor(uint16_t div);

uint64_t pit_get_frequency();

void pit_set_frequency(uint64_t frequency);

void pit_tick();
