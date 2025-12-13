#include <types.hh>
#include <gfx/console.hh>
#include <mm/pmm.hh>
#include <mm/vmm.hh>
#include <pci/pci.hh>
#include <acpi/acpi.hh>
#include <arch/arch.hh>
#include <arch/amd64/paging.hh>
#include <arch/amd64/amd64.hh>
#include <util/storage.hh>
#include <devmgr/module.hh>
#include <util/stacktrace.hh>
#include <util/ksyms.hh>
#include <test.hh>
#include <cppcompat.hh>
#include <devmgr/devmgr.hh>
#include <devmgr/usb/usb.hh>

extern "C" noret void khang();

// TODO: libk reform
// TODO: operator[] meg hasonlók küldjenek vissza Opt<T&>-t a T& helyett
// TODO: hashmap legyen a Map<K, V>
// TODO: csomó mindenhez csak 4k kell, nem 2m mint ahogy azt a pmm csinálja
// TODO: modulkód rw-ként van megadva, az adat meg execute-ként
// TODO: ELF fájlok feldolgozó kódja többször van leírva (ksyms.cc, module.cc, userspace majd)
// TODO: fájlok összeolvasztása, rohadt sok van ahoz képest amit tud a kernel
// TODO: UTF-8 konzol

#include <util/cpuid.hh>

extern "C" void kmain() {
	// Korai inicializáció
	arch_init();
	arch_read_boot_info();
	pmm_init();
	arch_late_init();
	vmm_init();

	cpp_construct_objects();

	con_init(FONTFILE_START);

	test();

	ksyms_read();

	modules_register_all();

	acpi_init();
	pci_init();

	usb_init_all();

	printk("Heap @ %p [%lld MiB]\n\r", pmm_heap_base, bytes2mibs(pmm_heap_size));
	printk(
		"Framebuffer %p: %dx%dx%d; font %dx%d; %llu MiBs; Heap: at %llu MiB, of size %llu MiB\n",
		fbs[0].fb_addr, fbs[0].fb_width, fbs[0].fb_height, fbs[0].fb_bpp,
		con_glyphw, con_glyphh,
		bytes2mibs(pmm_freemem + pmm_usedmem + pmm_reservedmem),
		bytes2mibs((u64)pmm_heap_base), bytes2mibs(pmm_heap_size)
	);

	printk("Mem usage:\n");
	printk(
		"VMM: %lld KiB free; %lld KiB used; %lld MiB total\n",
		bytes2kibs(vmm_freemem), bytes2kibs(vmm_usedmem), bytes2mibs(vmm_freemem + vmm_usedmem)
	);
	printk(
		"PMM: %lld KiB free; %lld KiB used; %lld MiB total\n",
		bytes2kibs(pmm_freemem), bytes2kibs(pmm_usedmem), bytes2mibs(pmm_freemem + pmm_usedmem)
	);

	printk("End of kmain()\n");

	khang();
}

noret void khang() {
	while (1) {
		arch_halt();
	}
}
