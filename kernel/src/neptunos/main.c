#include <neptunos/kutil.h>
#include <neptunos/libk/stdalign.h>

uint8_t kmain(system_info_t* _info) {
	kinit(_info);
	printk("Free: %ud MiB; Used: %ud KiB; Reserved: %ud MiB\n", free_mem / 1024 / 1024, used_mem / 1024, reserved_mem / 1024 / 1024);
	
	void* stack_base = malloc(0x10000); // 64 KiB
	asm("movq %0, %%rsp" :: "r" (stack_base));

	#ifdef USE_DOUBLE_BUFFERING
		sync_back_buffer();
	#endif

	//get_table_address("MCFG");

	text_color_push(0x0000ff00);
	printk("Successfully reached end of kmain, jumping to idle mode...\n");
	text_color_pop();

	// Halt until next interrupt
	while(1) asm("hlt");
	printk("Idle loop returned, halting...\n");
	halt();

	return 0;
}
