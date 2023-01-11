#include <neptunos/graphics/text_renderer.h>
#include <neptunos/libk/string.h>
#include <neptunos/globals.h>

#include <neptunos/libk/stdarg.h>

uint32_t color_stack, current_color = 0xd8d8d8d8;

uint16_t cursor_x = 0, cursor_y = 0;

#include <neptunos/libk/stdint.h>

const char xdigits[16] = {
	"0123456789ABCDEF"
};

const char* fmt_x(char* str, uint64_t value, uint8_t length);

void render_char(char c) {
	if (c == '\0') return;

	if(c == '\n') {
		cursor_y += 16;
		cursor_x = 0;
		return;
	} else if (c == '\t') {
		cursor_x += (info->g_info->info->width / 16) - (cursor_x % (info->g_info->info->width / 16));
		return;
	} else if (c == '\b') {
		cursor_x -= def->width;
		for (uint32_t y = cursor_y; y < cursor_y + def->height; y++) {
			for (uint32_t x = cursor_x; x < cursor_x + def->width; x++) {
				pixel(x, y, 0x00000000);
			}
		}
		return;
	}

    uint8_t* fontPtr = (uint8_t*)(glyph_buffer + (c * def->bytesperglyph));

    for (uint32_t y = cursor_y; y < cursor_y + def->height; y++) {
        for (uint32_t x = cursor_x; x < cursor_x + def->width; x++) {
			if ((*fontPtr & (0b10000000 >> (x - cursor_x))) > 0) {
				pixel(x, y, current_color);
			}
        }
        fontPtr++;
    }

	cursor_x += def->width;
	if (cursor_x >= info->g_info->info->width) {
		cursor_y += def->height;
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

/*__attribute__((no_caller_saved_registers))*/ void printk(char *restrict fmt, ...) {
    va_list arg_list;
    int16_t length = 0;

    va_start(arg_list, fmt);

    while(*fmt != '\0') {
        if (*fmt == '%') {
            fmt++;
			char res[256] = "";
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
				case 'p': {
						char testbuf[13];
						fmt_x(testbuf, (uint64_t)va_arg(arg_list, void*), 11);
						render_string(testbuf);
					}
					break;
            }
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

const char* fmt_x(char* str, uint64_t value, uint8_t length) {
	uint64_t* valPtr = &value;
	uint8_t* ptr;
	uint8_t tmp;
	for (uint8_t i = 0; i < length; i++){
		ptr = ((uint8_t*)valPtr + i);
		tmp = ((*ptr & 0xF0) >> 4);
		str[length - (i * 2 + 1)] = tmp + (tmp > 9 ? 55 : '0');
		tmp = ((*ptr & 0x0F));
		str[length - (i * 2)] = tmp + (tmp > 9 ? 55 : '0');
	}
	str[length + 1] = '\0';
	return str;
}
