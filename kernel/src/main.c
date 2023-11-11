#include <lib/kinfo.h>
#include <lib/sprintf.h>
#include <init/multiboot.h>
#include <serial/serial.h>
#include <graphics/console.h>
#include <arch/x86/power.h>
#include <memory/paging/paging.h>

kernel_info_t* info;

u8 kmain(kernel_info_t* _info) {
	info = _info;

	paging_init();
	info->kernel_addr = paging_lookup(0xfffffff800000000);
	// Multiboot tag-ek feldolgozása
	mb_parse_tags(info->mb_hdr_addr);

	printk("Hello world!\n");
	printk("Felbontas: %d x %d\n", fb.width, fb.height);

	// for (u64 i = 0; i < 800000000; i++) {
	// 	u8* asd = (u8*)pmm_alloc_page();
	// 	*asd = 0xff;
	// 	sprintk("%p\n", asd);
	// }

	// A processzor pihenhet a következő interruptig
	while(1) halt();

	return 88;
}
