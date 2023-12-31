#include <lib/kinfo.h>
#include <lib/sprintf.h>
#include <lib/symlookup.h>

#include <init/init.h>

#include <serial/serial.h>

#include <graphics/console.h>
#include <graphics/panic.h>

#include <arch/x86/power.h>
#include <arch/x86/gdt.h>
#include <arch/x86/idt.h>

#include <memory/paging/paging.h>
#include <memory/heap/vmm.h>

kernel_info_t* info;

void f1() { panic(""); }
void f2() { f1(); }
void f3() { f2(); }
void f4() { f3(); }
void f5() { f4(); }
void f6() { f5(); }
void f7() { f6(); }
void f8() { f7(); }

u8 kmain(kernel_info_t* _info) {
	info = _info;

	paging_init();
	info->kernel_addr = paging_lookup_2m(0xfffffff800000000);
	// Multiboot tag-ek feldolgozása
	mb_parse_tags(info->mb_hdr_addr);

	vmm_init();

	gdt_init();
	idt_init();

	f8();

	printk("Hello world!\n");
	printk("Felbontas: %d x %d\n", fb.width, fb.height);

	// A processzor pihenhet a következő interruptig
	while(1) halt();

	return 88;
}
