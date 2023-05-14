#include <neptunos/kutil.h>

// Experimnetal memcpy (WIP)
void* memcpyx(void* p, const void* src, size_t n) {
	// Move data using ymm0 (256-bit reg)
	if(n > 32) {
		__asm__ volatile("movups %0, %%xmm0" :: "m"(p));
	}

	return p;
}

uint8_t kmain(boot_info_t* _inf) {
	info = _inf;
	kinit();
	#ifdef USE_DOUBLE_BUFFERING
		sync_back_buffer();
	#endif

	

	printk("Free: %ud MiB; Used: %ud MiB; Reserved: %ud MiB\n", bytes_mib(free_mem), bytes_mib(used_mem), bytes_mib(reserved_mem));
	text_color_push(0x0000ff00);
	printk("Successfully reached end of kmain, jumping to idle mode...\n");
	text_color_pop();
	while(1);
	return 0;
}
