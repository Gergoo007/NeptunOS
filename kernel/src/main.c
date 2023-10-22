#include <lib/kinfo.h>
#include <lib/sprintf.h>
#include <init/multiboot.h>
#include <serial/serial.h>
#include <graphics/console.h>
#include <arch/x86/power.h>

u8 kmain(kernel_info_t* info) {
	// Multiboot tag-ek feldolgozása
	mb_parse_tags(info->mb_hdr_addr);

	con_init(0xffffffff, 0xff180101);

	printk("Hello world!\n");
	printk("Felbontas: %d x %d\n", fb.width, fb.height);

	// A processzor elaludhat a következő interruptig
	while(1) halt();

	return 88;
}
