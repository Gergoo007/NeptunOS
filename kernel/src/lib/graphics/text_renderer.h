#pragma once

#include "graphics.h"

#include <stdint.h>
#include <stdarg.h>

extern void render_char(char c);

extern void render_string(char* str);

extern /*__attribute__((no_caller_saved_registers))*/ void printk(char *fmt, ...);

extern void text_color(uint32_t color);

extern void text_color_pop();

extern void text_color_push(uint32_t color);

extern void text_color_reset();

extern const char* fmt_x(char* str, uint64_t value, uint8_t length);
