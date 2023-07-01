#include <neptunos/globals.h>
#include <neptunos/graphics/text_renderer.h>

screen_props_t screen;
boot_info_t* info = NULL;

void __stack_chk_fail(void) {
	serial_write(0x3f8, "!!-- Stack smashing detected --!!\n");
	panic("Stack smashing detected!\n");
}
