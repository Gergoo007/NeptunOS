#include <neptunos/globals.h>
#include <neptunos/graphics/text_renderer.h>

bitmap_t* bm;
void* mem_base = NULL;
system_info_t* info;

void __stack_chk_fail() {
	panic("Stack smashing detected!\n");
}
