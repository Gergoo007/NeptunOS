#include <neptunos/kutil.h>

extern void BSS_START;
extern void BSS_END;

// char buf[0x1000];
// char dest[0x1000];

uint8_t kmain(boot_info_t* _inf) {
	memset(&BSS_START, 0, &BSS_END - &BSS_START);
	info = _inf;
	kinit();

	#ifdef USE_DOUBLE_BUFFERING
		sync_back_buffer();
	#endif

	printk("SSE: %d\n", cpuid_sse());
	printk("AVX2: %d\n", cpuid_avx2());

	printk("Comp memcpy: %p\n", memcpy_comp);
	printk("SSE memcpy: %p\n", memcpy_sse);
	printk("AVX2 memcpy: %p\n", memcpy_avx2);
	printk("Current memcpy: %p\n", memcpy);

	char* buf = request_pages(100);
	char* dest = request_pages(100);
	printk("memset...\n");
	memset(buf, 1, 0x100000);
	printk("memcpy...\n");
	for (u32 i = 0; i < 1000; i++) {
		memcpy(dest, buf, 0x100000);
	}
	printk("memcpy done\n");

	printk("Total: %d MiB\nFree/Used: %d MiB/%d MiB\n", bytes_mib(total_mem), bytes_mib(free_mem), bytes_mib(used_mem));
	text_color_push(0x0000ff00);
	printk("Successfully reached end of kmain, jumping to idle mode...\n");
	text_color_pop();
	while(1);
	return 0;
}
