#include <lib/kinfo.h>
#include <lib/sprintf.h>
#include <lib/symlookup.h>

#include <multiboot/parser.h>

#include <serial/serial.h>

#include <graphics/console.h>
#include <graphics/panic.h>

#include <arch/x86/power.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>

#include <memory/paging/paging.h>
#include <memory/heap/vmm.h>

#include <apic/pit/pit.h>

#include <ps2/kbd.h>

kernel_info_t* info;

// TODO: konzisztens struct nevezés, typedef meg a '_t' eltávolítása
// TODO: boot előtt lehessen választani melyik driver vagy akármi logoljon info-t
// TODO: normális paging, bugmentes procedúrákkal, identity paging elhagyása
// TODO: vmm-hez normális malloc meg calloc

extern hba_port* tmp;

u8 kmain(kernel_info_t* _info) {
	info = _info;

	paging_init();
	info->kernel_addr = paging_lookup_2m(0xfffffff800000000);
	// Multiboot tag-ek feldolgozása
	mb_parse_tags(info->mb_hdr_addr);

	for (u8 i = 0; i < num_usb_devs; i++) {
		printk("dev %d: %s: %s\n", i, usb_devs[i].manufacturer, usb_devs[i].product);
	}

	ps2_kbd_init();
	
	printk("Hello world!\n");
	printk("Felbontas: %d x %d\n", fb.width, fb.height);

	void* out = pmm_alloc_page();
	printk("Olvasas %p-bol\n", tmp);
	ahci_read(tmp, 0, 1, out);

	printk("out: %s\n", out);

	// A processzor pihenhet a következő interruptig
	while(1) halt();

	return 88;
}
