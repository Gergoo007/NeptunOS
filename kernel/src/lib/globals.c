#include "globals.h"
#include "graphics/text_renderer.h"

bitmap* bm;
void* mem_base = NULL;

void init_globals() {
	
}

void __stack_chk_fail() {
	printk("!! Stack smashing detected !!");
}