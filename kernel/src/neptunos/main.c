#include <neptunos/kutil.h>

uint8_t kmain(system_info_t* _info) {
	kinit(_info);
	printk("Free: %ud MiB; Used: %ud KiB; Reserved: %ud MiB\n", free_mem / 1024 / 1024, used_mem / 1024, reserved_mem / 1024 / 1024);
	
	#ifdef USE_DOUBLE_BUFFERING
		sync_back_buffer();
	#endif

	text_color_push(0x0000ff00);
	printk("Successfully reached end of kmain, jumping to idle mode...\n");
	text_color_pop();
	
	
	
	printk("Idle loop returned, halting...\n");
	halt();

	return 0;
}
