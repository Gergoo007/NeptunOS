#include <neptunos/graphics/text_renderer.h>

uint32_t color_stack, current_color = 0xd8d8d8d8;
uint32_t background_color = 0x00000000;

uint16_t cursor_x = 0, cursor_y = 0;

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
				pixel(x, y, background_color);
			}
		}
		return;
	}

	uint8_t* font_ptr = (uint8_t*)(glyph_buffer + (c * def->bytesperglyph));

	for (uint32_t y = cursor_y; y < cursor_y + def->height; y++) {
		for (uint32_t x = cursor_x; x < cursor_x + def->width; x++) {
			if ((*font_ptr & (0b10000000 >> (x - cursor_x))) > 0) {
				pixel(x, y, current_color);
			} else {
				pixel(x, y, background_color);
			}
		}
		font_ptr++;
	}

	cursor_x += def->width;
	if (cursor_x >= info->g_info->info->width) {
		cursor_y += def->height;
		cursor_x = 0;
	}
}

void render_char_transparent(char c) {
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

	uint8_t* font_tr = (uint8_t*)(glyph_buffer + (c * def->bytesperglyph));

	for (uint32_t y = cursor_y; y < cursor_y + def->height; y++) {
		for (uint32_t x = cursor_x; x < cursor_x + def->width; x++) {
			if ((*font_tr & (0b10000000 >> (x - cursor_x))) > 0) {
				pixel(x, y, current_color);
			}
		}
		font_tr++;
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

void printk(char *restrict fmt, ...) {
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
						char testbuf[17];
						fmt_x(testbuf, (uint64_t)va_arg(arg_list, void*), 15, 1);
						render_string(testbuf);
					}
					break;
				default: {
						if (*fmt <= '9' && *fmt >= '0') {
							uint8_t first = (uint8_t)(*fmt++) - (uint8_t)'0';
							uint8_t second = (uint8_t)*(fmt++) - (uint8_t)'0';
							uint8_t final = second + (first * 10);
							
							switch (*fmt) {
								case 'x': {
									char buffer[final + 1];
									memset(buffer, 0, final + 1);
									fmt_x(buffer, (uint64_t)va_arg(arg_list, uint64_t), final, 0);
									render_string(buffer);
									break;
								}
								case 'X': {
									char buffer[final + 1];
									memset(buffer, 0, final + 1);
									fmt_x(buffer, (uint64_t)va_arg(arg_list, uint64_t), final, 1);
									render_string(buffer);
									break;
								}
							}
						}
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

void printk_serial(char *restrict fmt, ...) {
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
					serial_write_c(0x3f8, (char) va_arg(arg_list, uint32_t));
					break;
				case 'd':
					serial_write(0x3f8, _int_to_str(va_arg(arg_list, int64_t), res, 10));
					break;
				case 'u':
					fmt++;
					switch(*fmt) {
						case 'd':
							serial_write(0x3f8, __int_to_str(va_arg(arg_list, uint64_t), res, 10));
							break;
					}
					break;	
				case 's':
					printk_serial(va_arg(arg_list, char*));
					break;
				case 'p': {
						char testbuf[17];
						fmt_x(testbuf, (uint64_t)va_arg(arg_list, void*), 15, 1);
						serial_write(0x3f8, testbuf);
					}
					break;
			}
		} else {
			serial_write_c(0x3f8, *fmt);
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

const char* fmt_x(char* str, uint64_t value, uint8_t length, uint8_t upper_case) {
	const char characters[16] = "0123456789abcdef";
	const char characters_upper[16] = "0123456789ABCDEF";

	if (upper_case) {
		for (uint8_t i = 0; i < length; i++) {
			char character = characters_upper[value % 16];
			str[length - i - 1] = character;
			value /= 16;
		}
		str[length] = '\0';
		return str;
	} else {
		for (uint8_t i = 0; i < length; i++) {
			char character = characters[value % 16];
			str[length - i - 1] = character;
			value /= 16;
		}
		str[length] = '\0';
		return str;
	}
}
