#include <neptunos/kutil.h>
#include <neptunos/libk/stdalign.h>

uint8_t kmain(system_info_t* _info) {
	kinit(_info);
	#ifdef USE_DOUBLE_BUFFERING
		sync_back_buffer();
	#endif

	
	
	printk("Free: %ud MiB; Used: %ud MiB; Reserved: %ud MiB\n", bytes_mib(free_mem), bytes_mib(used_mem), bytes_mib(reserved_mem));
	text_color_push(0x0000ff00);
	printk("Successfully reached end of kmain, jumping to idle mode...\n");
	text_color_pop();

	// Halt until next interrupt
	while(1) asm("hlt");
	printk("Idle loop returned, halting...\n");
	halt();

	return 0;
}
