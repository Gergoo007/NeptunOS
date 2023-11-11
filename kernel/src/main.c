#include <lib/kinfo.h>
#include <lib/sprintf.h>
#include <init/multiboot.h>
#include <serial/serial.h>
#include <graphics/console.h>
#include <arch/x86/power.h>

kernel_info_t* info;

u8 kmain(kernel_info_t* _info) {
	info = _info;
	// Multiboot tag-ek feldolgozása
	mb_parse_tags(info->mb_hdr_addr);

	printk("Hello world!\n");
	printk("Felbontas: %d x %d\n", fb.width, fb.height);
	
	// for (u16 i = 0; i < (1 << 16)-1; i++) {
	// 	printk("Free page: %p\n\r", pmm_alloc_page());
	// }

	// A processzor pihenhet a következő interruptig
	while(1) halt();

	return 88;
}
