#include <neptunos/kutil.h>

uint8_t kmain(system_info* _info) {
	kinit(_info);
	printk("Free: %ud MiB; Used: %ud KiB; Reserved: %ud MiB\n", free_mem / 1024 / 1024, used_mem / 1024, reserved_mem / 1024 / 1024);

	//render_char((char)0xcf);
	//render_char('_');

	#ifdef USE_DOUBLE_BUFFERING
		sync_back_buffer();
	#endif

	while (1);
	// text_color_push(0x0000ff00);
	// printk("Successfully reached end of kmain, halting...");
	// text_color_pop();
	//halt();

	return 0;
}
