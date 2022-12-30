#include "lib/kutil.h"

uint8_t kmain(system_info* _info) {
	// asm("mov $0xffffffffffffffff, %rsp");
	//asm("mov %rsp, %rbp");
	kinit(_info);
	printk("Free: %ud MiB; Used: %ud MiB; Reserved: %ud MiB\n", free_mem / 1024 / 1024, used_mem / 1024 / 1024, reserved_mem / 1024 / 1024);

	void* first = request_page();
	void* second = request_page();
	void* third = request_page();
	printk("Testing request page:\n%p\n%p\n%p\n", first, second, third);

	pml4 = (page_map_level*) request_page();
	memset(pml4, 0, 0x1000);

	printk("Total mem: %ud MiB\n", total_mem / 1024 / 1024);
	for (uint64_t i = 0; i < total_mem; i += 0x1000) {
		map_address((void*)i, (void*)i);
	}

	// // Identity map _info
	// for (uint64_t i = (uint64_t)_info; i < (uint64_t)_info+sizeof(system_info)+1; i+=0x1000) {
	// 	map_address((void*) i, (void*) i);
	// }

	// printk("First\n");

	// // Identity map info
	// for (uint64_t i = (uint64_t)info; i < (uint64_t)info+sizeof(system_info)+1; i+=0x1000) {
	// 	map_address((void*) i, (void*) i);
	// }
	// printk("Second\n");

	// // Identity map the kernel
	// for (uint64_t i = (uint64_t)&KSTART; i < (uint64_t)&KSTART+(uint64_t)&KEND; i+=0x1000) {
	// 	map_address((void*) i, (void*) i);
	// }
	// printk("Third\n");

	// Identity map the framebuffer
	// for (uint64_t i = (uint64_t)info->g_info->fb_base; i < (uint64_t)info->g_info->fb_base+(uint64_t)info->g_info->fb_size; i+=0x1000) {
	// 	map_address((void*) i, (void*) i);
	// }
	// printk("Fourth\n");
	// // Identity map g_info
	// for (uint64_t i = (uint64_t)info->g_info; i < (uint64_t)info->g_info+sizeof(efi_gop_mode_t)+1; i+=0x1000) {
	// 	map_address((void*) i, (void*) i);
	// }
	// printk("Fifth\n");
	// // Identity map fb mode info
	// for (uint64_t i = (uint64_t)info->g_info->info; i < (uint64_t)info->g_info->info+sizeof(efi_gop_mode_info_t)+1; i+=0x1000) {
	// 	map_address((void*) i, (void*) i);
	// }
	// printk("Sixth\n");
	// map_address(void *virtual_address, void *physical_address)

	// printk("Page Table: %ud\nPage Dir: %ud\nPage Dir Pointers: %ud\nPML4: %ud\n", 
	// 	sizeof(page_table), sizeof(page_dir), sizeof(page_dir_pointers), sizeof(page_map_level_4));

	// printk("Entry: %ud\n", sizeof(page_map_entry));

	// serial_write(0x3f8, "ALIVE");

	// map_address((void*)0x0000, (void*)0x0000000000001000);
	// asm("mov %%cr3, %0" : "=r" (pml4));

	for (uint64_t i = 0; i <= total_mem; i += 0x1000) {
		map_address((void*)i, (void*)i);
	}

	for (uint64_t i = (uint64_t)(info->g_info->fb_base); i <= (uint64_t)(info->g_info->fb_base) + info->g_info->fb_size; i += 0x1000) {
		map_address((void*)i, (void*)i);
	}
	
	asm("mov %0, %%cr3" :: "r" (pml4));
	printk("Paging is working!!!\n");

	text_color_push(0x0000ff00);
	printk("Successfully reached end of kmain, halting...");
	text_color_pop();
	asm("cli");
	asm("hlt");
	halt();	
	printk("Halt unsuccessful.\n");

	return 0;
}
