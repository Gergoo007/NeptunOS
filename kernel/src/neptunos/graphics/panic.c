#include <neptunos/graphics/panic.h>

extern void _printk(char* restrict fmt, ...);

_attr_no_caller_saved_regs void _panic(char msg[256], char* caller, char* file, uint32_t line, ...) {
	va_list args;
	va_start(args, line);
	text_color(0x00ff0000);
	cursor_x = 0;
	printk("===PANIC===\n");
	printk("- build: %s %s\n", __DATE__, __TIME__);
	printk("- caller: %s @ %s L%d\n", caller, file, line);
	printk("- message: ");
	_printk(msg, args);
	asm("cli");
	asm("hlt");
	while (1);
}
