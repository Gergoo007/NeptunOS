#include <neptunos/graphics/panic.h>

_attr_no_caller_saved_regs void _panic(char msg[256], char* caller, char* file, uint32_t line) {
	text_color_push(0x00ff0000);
	cursor_x = 0;
	printk("===PANIC===\n");
	printk("- build: %s %s\n", __DATE__, __TIME__);
	printk("- caller: %s @ %s L%d\n", caller, file, line);
	printk("- message: %s\n", msg);
	text_color_pop();
}
