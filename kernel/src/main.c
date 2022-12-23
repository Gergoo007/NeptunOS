#include "lib/globals.h"

#include "lib/graphics/graphics.h"

#include "lib/graphics/text_renderer.h"
#include "lib/serial/serial.h"
#include "lib/libk/string.h"

#include "lib/memory/memory.h"

BootInfo* graphics;
uint16_t cursor_x = 0, cursor_y = 0;

uint8_t kmain(BootInfo *restrict _graphics) {
    graphics = _graphics;

	clear_screen();

	text_color_push(0x0000ffcc);
	printk("Framebuffer resolution: %ud x %ud\n", graphics->framebuffer->Width, graphics->framebuffer->Height);
	text_color_pop();

	printk("Kernel size: %ud KiB\n", ((uint64_t) &KEND - (uint64_t) &KSTART) / 1024);

	serial_init();
	init_globals();

	map_memory();
	init_bitmap(bm);

	void* test_obj = request_page();
	void* test_obj_two = request_page();
	printk("Address of test_obj: \t\t%p\n", test_obj);
	printk("Address of test_obj_two: \t%p\n", test_obj_two);
	
	//printk("Hex testtttt: %p\n", &test_obj);

	printk("Total mem: %ud MiB or %ud GiB (%ud pages)\n", total_mem / 1024 / 1024, total_mem / 1024 / 1024 / 1024, num_pages);

	text_color(0x0000ff00);
	printk("Successfully reached end of kernel kmain(), halting...\n");
	
	asm("cli");
	asm("hlt");
	printk("Halt unsuccessful.\n");

	return 1;
}
