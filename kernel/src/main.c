#include "lib/kutil.h"

uint8_t kmain(system_info* _info) {
	kinit(_info);
	printk("Free: %ud MiB; Used: %ud MiB; Reserved: %ud MiB\n", free_mem / 1024 / 1024, used_mem / 1024 / 1024, reserved_mem / 1024 / 1024);

	

	text_color_push(0x0000ff00);
	printk("Successfully reached end of kmain, halting...");
	text_color_pop();
	halt();	
	printk("Halt unsuccessful.\n");

	return 0;
}
