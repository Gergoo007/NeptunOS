#include <neptunos/graphics/text_renderer.h>

u32 color_stack = 0x00000000;
u32 background_color = 0x00000000;
u32 current_color = 0xd8d8d8d8;

u16 cursor_x = 0;
u16 cursor_y = 0;

extern void _printk(const char *restrict fmt, va_list arg_list);

void render_char(char c) {
	if (c == '\0') return;

	if(c == '\n') {
		cursor_y += 16;
		cursor_x = 0;
		return;
	} else if (c == '\t') {
		cursor_x += (screen.width / 16) - (cursor_x % (screen.width / 16));
		return;
	} else if (c == '\b') {
		cursor_x -= def.width;
		for (uint32_t y = cursor_y; y < cursor_y + def.height; y++) {
			for (uint32_t x = cursor_x; x < cursor_x + def.width; x++) {
				pixel(x, y, background_color);
			}
		}
		return;
	}

	uint8_t* font_ptr = (uint8_t*)((u8*)glyph_buffer + (c * def.bytesperglyph));

	for (uint32_t y = cursor_y; y < cursor_y + def.height; y++) {
		for (uint32_t x = cursor_x; x < cursor_x + def.width; x++) {
			if ((*font_ptr & (0b10000000 >> (x - cursor_x))) > 0) {
				pixel(x, y, current_color);
			} else {
				pixel(x, y, background_color);
			}
		}
		font_ptr++;
	}

	cursor_x += def.width;
	if (cursor_x >= screen.width) {
		cursor_y += def.height;
		cursor_x = 0;
	}
}

void render_string(const char *restrict str) {
	while(*str != '\0') {
		render_char(*str);
		str++;
	}
}

// Normal printk
void printk(const char *restrict fmt, ...) {
	va_list args;
	va_start(args, fmt);
	_printk(fmt, args);
	va_end(args);
}

void _printk(const char *restrict fmt, va_list arg_list) {
	while(*fmt != '\0') {
		if (*fmt == '%') {
			fmt++;
			switch (*fmt) {
				case 'c':
					render_char((char) va_arg(arg_list, uint32_t));
					break;
				case 'd': {
					char res[256] = "";
					render_string(int64_to_str(va_arg(arg_list, int64_t), res, 10));
					break;
				}
				case 'u':
					fmt++;
					switch(*fmt) {
						case 'd': {
							char res[256] = "";
							render_string(uint_to_str(va_arg(arg_list, uint64_t), res, 10));
							break;
						}
					}
					break;
				case 's':
					printk(va_arg(arg_list, char*));
					break;
				case 'p': {
					char testbuf[17];
					fmt_x(testbuf, (uint64_t)va_arg(arg_list, void*), 16, 1);
					render_string(testbuf);
					break;
				}
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
								case 's': {
									char buffer[final + 1];
									memcpy(buffer, (void*)va_arg(arg_list, char*), final);
									buffer[final] = '\0';
									render_string(buffer);
									break;
								}
								case 'b': {
									uint64_t num = va_arg(arg_list, uint64_t);
									while (final-- > 0) {
										if (num & (1 << final))
											render_char('1');
										else
											render_char('0');
									}
									break;
								};
								default:
									render_string("Invalid format: ");
									render_char(*fmt);
									render_string("!\n");
									break;
							}
						}
					}
					break;
			}
		} else {
			render_char(*fmt);
		}

		fmt++;

		if (cursor_y >= screen.height) {
			cursor_y = 0;
		}
	}
}

void _report(const char* msg, char* filename, ...) {
	va_list list;
	va_start(list, filename);
	text_color_push(0x00fff000);
	render_string("[DEBUG] ");
	render_char('[');
	render_string(filename);
	render_string("] ");
	_printk(msg, list);
	text_color_pop();
	va_end(list);
}

void text_color(uint32_t color) {
	current_color = color;
}

void text_color_pop(void) {
	current_color = color_stack;
}

void text_color_push(uint32_t color) {
	color_stack = current_color;
	current_color = color;
}

void text_color_reset(void) {
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
