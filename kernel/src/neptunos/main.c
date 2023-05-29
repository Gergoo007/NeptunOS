#include <neptunos/kutil.h>

extern void BSS_START;
extern void BSS_END;

uint8_t kmain(boot_info_t* _inf) {
	memset(&BSS_START, 0, &BSS_END - &BSS_START);
	info = _inf;
	kinit();

	#ifdef USE_DOUBLE_BUFFERING
		sync_back_buffer();
	#endif

	printk("Total: %d MiB\nFree/Used: %d MiB/%d MiB\n", bytes_mib(total_mem), bytes_mib(free_mem), bytes_mib(used_mem));
	text_color_push(0x0000ff00);
	printk("Successfully reached end of kmain, jumping to idle mode...\n");
	text_color_pop();
	while(1);
	return 0;
}
