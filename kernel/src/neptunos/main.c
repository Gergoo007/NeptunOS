#include <neptunos/kutil.h>
#include <neptunos/libk/stdalign.h>

uint8_t kmain(system_info_t* _info) {
	kinit(_info);
	printk("Free: %ud MiB; Used: %ud KiB; Reserved: %ud MiB\n", free_mem / 1024 / 1024, used_mem / 1024, reserved_mem / 1024 / 1024);
	
	#ifdef USE_DOUBLE_BUFFERING
		sync_back_buffer();
	#endif

	text_color_push(0x0000ff00);
	printk("Successfully reached end of kmain, jumping to idle mode...\n");
	text_color_pop();

	// char test[256] = "";
	// fmt_x(test, 0x1234567abcdef, 13, 0);
	// printk("\n%s\n", test);
	
	// uint64_t aaa;
	// printk("Address of aaa: %p\n", &aaa);
	
	// align(0x1000) uint64_t aligned;
	// printk("Address of aaa: %p\n", &aligned);
	// //printk("Alignment: %05x\n", alignof(uint64_t));

	printk("HHHH: %03x", 0xfa);
	
	while(1);
	printk("Idle loop returned, halting...\n");
	halt();

	return 0;
}
