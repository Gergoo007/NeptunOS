#include "globals.h"
#include "graphics/text_renderer.h"

uint64_t bitmap_size;

void init_globals() {
	
}

void __stack_chk_fail() {
	printk("!! Stack smashing detected !!");
}