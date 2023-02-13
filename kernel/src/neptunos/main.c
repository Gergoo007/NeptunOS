#include <neptunos/kutil.h>
#include <neptunos/libk/stdalign.h>

//
// Plan for paging: Copy the whole kernel, environment, bootboot structure
// into a known physical address, then map them
//

uint8_t kmain() {
	if (bootboot.bspid) {
		
	}

	kinit();
	#ifdef USE_DOUBLE_BUFFERING
		sync_back_buffer();
	#endif

	printk("FB: %p\nBOOTBOOT: %p\nMMIO: %p\nENVIRONMENT: %p\n", &fb, &bootboot, &mmio, &environment);
	printk("%p\n%p\n", &KSTART, &KEND);
	printk("%p\n%p\n", KSTART, KEND);

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
