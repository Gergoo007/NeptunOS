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

extern "C" noret void khang();

// TODO: nem jó már a limine submodule
// TODO: UTF-8 konzol

extern "C" void kmain() {
	// Korai inicializáció
	arch::init();
	arch::read_boot_info();
	console::init(FONTFILE_START);
	pmm::init();
	arch::late_init();
	vmm::init();

	// Itt már az alapvető rendszerek működnek
	cpp_construct_objects();

	acpi::init();
	pci::init();

	modules::register_all();

	printk(
		"Framebuffer: %dx%dx%d; font %dx%d; %llu MiBs; Heap: at %llu MiB, of size %llu MiB\n",
		machine.fbs[0].fb_width, machine.fbs[0].fb_height, machine.fbs[0].fb_bpp,
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
