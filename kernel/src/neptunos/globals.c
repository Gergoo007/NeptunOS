#include <neptunos/globals.h>
#include <neptunos/graphics/text_renderer.h>

bitmap_t bm;
screen_props_t screen;
boot_info_t* info = NULL;

void __stack_chk_fail(void) {
	panic("Stack smashing detected!\n");
}
