#pragma once

#include <neptunos/graphics/graphics.h>

#include <neptunos/serial/serial.h>

// Debug printk
#define reportln(x, ...) _report(x, __FILE_NAME__, ##__VA_ARGS__); render_char('\n');

// Debug printk
#define report(x, ...) _report(x, __FILE_NAME__, ##__VA_ARGS__); render_char('\n');

// Fatal error printk
#define fatal(x, ...) text_color(0x00ff0000); render_string("[FATAL] "); printk(x, ##__VA_ARGS__); asm("cli"); halt();

// Non-fatal error printk
#define error(x, ...) text_color_push(0x00ff0000); render_string("[ERROR] "); printk(x, ##__VA_ARGS__); text_color_pop();

// Warning printk
#define warning(x, ...) text_color_push(0x00ff6600); render_string("[WARNING] "); printk(x, ##__VA_ARGS__); text_color_pop();

void render_char(char c);
void render_string(char* str);

void printk(char *fmt, ...);
void _report(char* msg, char* filename, ...);

void text_color(uint32_t color);
void text_color_pop();
void text_color_push(uint32_t color);
void text_color_reset();

const char* fmt_x(char* str, uint64_t value, uint8_t length, uint8_t upper_case);
