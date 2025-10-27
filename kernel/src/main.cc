#include <types.hh>
#include <arch/arch.hh>
#include <cppcompat.hh>
#include <gfx/console.hh>
#include <mm/pmm.hh>
#include <mm/vmm.hh>
#include <pci/pci.hh>
#include <acpi/acpi.hh>
#include <arch/amd64/paging.hh>
#include <util/storage.hh>
#include <devmgr/module.hh>
#include <util/ksyms.hh>

extern "C" noret void khang();

// TODO: libk reform
// TODO: operator[] meg hasonlók küldjenek vissza Opt<T&>-t a T& helyett
// TODO: hashmap legyen a Map<K, V>
// TODO: csomó mindenhez csak 4k kell, nem 2m mint ahogy azt a pmm csinálja
// TODO: modulkód rw-ként van megadva, az adat meg execute-ként
// TODO: ELF fájlok feldolgozó kódja többször van leírva (ksyms.cc, module.cc, userspace majd)
// TODO: kellenek a namespace-ek? buziság
// TODO: fájlok összeolvasztása, rohadt sok van ahoz képest amit tud a kernel
// TODO: UTF-8 konzol

#include <util/cpuid.hh>

extern "C" void kmain() {
	// Ehhez nem kell semmi se szinte
	// ksyms_read();

	// Korai inicializáció
	arch::init();
	arch::read_boot_info();
	pmm::init();
	arch::late_init();
	vmm::init();

	// Itt már az alapvető rendszerek működnek
	cpp_construct_objects();

	console::init(FONTFILE_START);

	ksyms_read();

	acpi::init();
	pci::init();

	modules::register_all();

	printk("Heap @ %p [%lld MiB]\n\r", pmm::heap_base, bytes2mibs(pmm::heap_size));
	printk(
		"Framebuffer %p: %dx%dx%d; font %dx%d; %llu MiBs; Heap: at %llu MiB, of size %llu MiB\n",
		machine.fbs[0].fb_addr, machine.fbs[0].fb_width, machine.fbs[0].fb_height, machine.fbs[0].fb_bpp,
		console::glyphw, console::glyphh,
		bytes2mibs(pmm::freemem + pmm::usedmem + pmm::reservedmem),
		bytes2mibs((u64)pmm::heap_base), bytes2mibs(pmm::heap_size)
	);

	printk("End of kmain()\n");

	khang();
}

extern "C" noret void khang() {
	while (1) {
		hlt();
	}
}
