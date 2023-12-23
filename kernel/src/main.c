#include <lib/kinfo.h>
#include <lib/sprintf.h>

#include <init/init.h>

#include <serial/serial.h>

#include <graphics/console.h>

#include <arch/x86/power.h>

#include <memory/paging/paging.h>
#include <memory/heap/vmm.h>

kernel_info_t* info;

u8 kmain(kernel_info_t* _info) {
	info = _info;

	paging_init();
	info->kernel_addr = paging_lookup(0xfffffff800000000);
	// Multiboot tag-ek feldolgozása
	mb_parse_tags(info->mb_hdr_addr);

	vmm_init();

	printk("Hello world!\n");
	printk("Felbontas: %d x %d\n", fb.width, fb.height);

	// VMM tesztelése
	u64* p = request_page();
	*p = 0x2000;
	printk("Allocated at %p\n", p);
	free_page(p);

	p = request_page();
	*p = 0x4000;
	printk("Allocated at %p\n", p);
	free_page(p);

	p = request_page();
	*p = 0x7000;
	printk("Allocated at %p\n", p);
	free_page(p);

	// A processzor pihenhet a következő interruptig
	while(1) halt();

	return 88;
}
