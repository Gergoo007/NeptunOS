#include "text_renderer.h"
#include "../libk/string.h"
#include "../globals.h"

#include <stdarg.h>

uint32_t color_stack, current_color = 0xd8d8d8d8;

static const char xdigits[16] = {
	"0123456789ABCDEF"
};

static char* fmt_x(uintmax_t x, char *s, int lower) {
	for (; x; x>>=4) *--s = xdigits[(x&15)]|lower;
	return s;
}

void render_char(char c) {
    char* fontPtr = (char*)graphics->psf1_Font->glyphBuffer + (c * graphics->psf1_Font->psf1_Header->charsize);

    for (unsigned long y = cursor_y; y < cursor_y + 16; y++) {
        for (unsigned long x = cursor_x; x < cursor_x + 8; x++) {
			if ((*fontPtr & (0b10000000 >> (x - cursor_x))) > 0) {
				pixel(x, y, current_color);
			}
        }
        fontPtr++;
    }

	cursor_x += 8;
	if (cursor_x >= graphics->framebuffer->Width) {
		cursor_y += 16;
		cursor_x = 0;
	}
}

void render_string(char *restrict str) {
	uint16_t length = 0;
    while(*str != '\0') {
		render_char(*str);
		str++;
		length++;
	}
}

void printk(char *restrict fmt, ...) {
    va_list arg_list;
    int16_t length = 0;

    va_start(arg_list, fmt);

    while(*fmt != '\0') {
        if (*fmt == '%') {
            fmt++;
			char* res = "";
			int _res = 0;
            switch (*fmt) {
                case 'c':
					render_char((char) va_arg(arg_list, uint32_t));
					break;
				case 'd':
					render_string(_int_to_str(va_arg(arg_list, int64_t), res, 10));
					break;
				case 'u':
					fmt++;
					switch(*fmt) {
						case 'd':
							render_string(__int_to_str(va_arg(arg_list, uint64_t), res, 10));
							break;
					}
					break;	
				case 's':
					printk(va_arg(arg_list, char*));
					break;
            }
        } else if(*fmt == '\n') {
			cursor_y += 16;
			cursor_x = 0;
		} else if (*fmt == '\t') {
			cursor_x += (graphics->framebuffer->Width / 16) - (cursor_x % (graphics->framebuffer->Width / 16));
		} else {
            render_char(*fmt);
            length++;
		}

        fmt++;
    }
}

void text_color(uint32_t color) {
	current_color = color;
}

void text_color_pop() {
	current_color = color_stack;
}

void text_color_push(uint32_t color) {
	color_stack = current_color;
	current_color = color;
}

void text_color_reset() {
	current_color = 0xd8d8d8d8;
}
