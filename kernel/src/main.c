#include "lib/kutil.h"

uint8_t kmain(system_info* _info) {
	// asm("mov $0xffffffffffffffff, %rsp");
	//asm("mov %rsp, %rbp");
	kinit(_info);
	printk("Free: %ud MiB; Used: %ud MiB; Reserved: %ud MiB\n", free_mem / 1024 / 1024, used_mem / 1024 / 1024, reserved_mem / 1024 / 1024);

	

	text_color_push(0x0000ff00);
	printk("Reached end of kmain successfully, halting...");
	text_color_pop();
	asm("cli");
	asm("hlt");
	halt();	
	printk("Halt unsuccessful.\n");

	return 0;
}
