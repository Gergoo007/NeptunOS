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

#include <storage/storage.h>

#include <loader/loader.h>

#include <arch/x86/syscalls/teszt.h>

kernel_info* kinfo;

// TODO: normális paging, bugmentes procedúrákkal, identity paging elhagyása
// TODO: vmm-hez normális malloc meg calloc
// TODO: (AHCI többek közt) MMIO caching kikapcsolása
// TODO: jobb ACPI kód
// TODO: parallel build Makefile
// TODO: Elrendezés

u8 kmain(kernel_info* _info) {
	kinfo = _info;

	paging_init();
	kinfo->kernel_addr = paging_lookup_2m(0xfffffff800000000);
	// Multiboot tag-ek feldolgozása
	mb_parse_tags(kinfo->mb_hdr_addr);

	for (u8 i = 0; i < num_usb_devs; i++) {
		printk("dev %d: %s: %s\n", i, usb_devs[i].manufacturer, usb_devs[i].product);
	}

	for (u8 i = 0; i < num_drives; i++) {
		printk("Meghajto #%d: %s: %s; %d GiB\n", i, storage_id(&drives[i]), drives[i].model, drives[i].size / 1024 / 1024 / 1024);
	}

	ps2_kbd_init();

	printk("Hello world!\n");
	printk("Felbontas: %d x %d\n", fb.width, fb.height);

	void* program = pmm_alloc_page();
	// QEMU HARDDISK megkeresése, azon van a program
	for (u8 i = 0; i < num_drives; i++) {
		if (strncmp(drives[i].model, "QEMU HARDDISK", 13)) continue;

		ahci_read(drives[i].port, 0, drives[i].size / 512, program);
	}

	void (*ent)(void) = loader_static(program);
	printk("Teszt elott\n");
	teszt(ent, pmm_alloc_page());
	printk("Teszt utan\n");

	// A processzor pihenhet a következő interruptig
	while(1) halt();

	return 88;
}
