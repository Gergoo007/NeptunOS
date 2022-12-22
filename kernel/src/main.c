#include "lib/globals.h"

#include "lib/graphics/graphics.h"

#include "lib/graphics/text_renderer.h"
#include "lib/serial/serial.h"
#include "lib/libk/string.h"

#include "lib/memory/memory.h"

BootInfo* graphics;
uint16_t cursor_x = 0, cursor_y = 0;

extern void* KSTART;
extern void* KEND;

void teszt(uint64_t address, uint64_t count) {
	uint64_t page_i = (uint64_t) address / 4096; // melyik page (hanyadik)
	uint64_t byte_i = page_i / 8;
	uint64_t bit_i = page_i % 8;
	//bitmap[byte_i] &= 1 << (bit_i);
	printk("page_i: %ud\nbyte_i: %ud\nbit_i: %ud\n", page_i, byte_i, bit_i);
	printk(byte_i > sizeof(bitmap) ? "byte_i nagyobb mint sizeof(bitmap)??\n" : "Minden rendben\n");
	printk("sizeof(bitmap): %ud vs %ud\n", bitmap_size, num_pages / 8);
}

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
	uint8_t _bitmap[(num_pages % 8 == 0) ? num_pages / 8 : (num_pages / 8) + 1];
	init_bitmap(_bitmap, sizeof(_bitmap));
	
	uint8_t i = 0;
	teszt((uint64_t) &i, 1);

	printk("Total mem: %ud MiB or %ud GiB (%ud pages)\n", total_mem / 1024 / 1024, total_mem / 1024 / 1024 / 1024, num_pages);

	text_color(0x0000ff00);
	printk("Successfully reached end of kernel kmain(), halting...\n");
	
	asm("cli");
	asm("hlt");
	printk("Halt unsuccessful.\n");

	return 1;
}
