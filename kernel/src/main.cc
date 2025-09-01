#include <types.hh>
#include <arch/arch.hh>
#include <cppcompat.hh>
#include <gfx/console.hh>
#include <mm/pmm.hh>
#include <mm/vmm.hh>
#include <pci/pci.hh>
#include <acpi/acpi.hh>
#include <arch/amd64/paging.hh>

extern "C" noret void khang();

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

	printk(
		"Framebuffer: %dx%dx%d; betu %dx%d; %llu MiBs; Heap itt: %llu MiB, ekkora: %llu MiB\n",
		g_info.fbs[0].fb_width, g_info.fbs[0].fb_height, g_info.fbs[0].fb_bpp,
		console::glyphw, console::glyphh,
		bytes2mibs(pmm::free + pmm::used + pmm::reserved),
		bytes2mibs((u64)pmm::heap_base), bytes2mibs(pmm::heap_size)
	);

	printk("ennyi\n");

	khang();
}

extern "C" noret void khang() {
	while (1) {
		asm volatile ("hlt");
	}
}
