#include <types.hh>
#include <gfx/console.hh>
#include <mm/pmm.hh>
#include <mm/pmm4g.hh>
#include <mm/vmm.hh>
#include <pci/pci.hh>
#include <acpi/acpi.hh>
#include <arch/arch.hh>
#include <arch/amd64/paging.hh>
#include <arch/amd64/amd64.hh>
#include <arch/amd64/cpuid.hh>
#include <arch/amd64/idt.hh>
#include <util/storage.hh>
#include <devmgr/module.hh>
#include <util/stacktrace.hh>
#include <util/ksyms.hh>
#include <test.hh>
#include <cppcompat.hh>
#include <devmgr/devmgr.hh>
#include <devmgr/usb/usb.hh>
#include <scheduler/scheduler.hh>

extern "C" noret void khang();

// TODO: int.s -> sse_state nem thread safe

namespace std { template <typename _Signature> class function {}; }

#include <util/helpers.hh>

extern "C" void kmain() {
	// Korai inicializáció
	arch_init(true);
	arch_read_boot_info();
	pmm_init();
	arch_late_init(true);
	vmm_init();

	cpp_construct_objects();

	con_init(FONTFILE_START);

	report("max leaf: %02x", cpuid_max_leaf());
	report("intel/amd %d %d", cpuid_is_intel(), cpuid_is_amd());

	if (!cpuid_x2apic_supported()) warn("TODO: x2APIC needed to function!");

	test();

	ksyms_read();

	sched_start();

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
		"VMM: %lld KiB free; %lld KiB used; %lld MiB total; num of allocs: %d\n",
		bytes2kibs(vmm_freemem), bytes2kibs(vmm_usedmem), bytes2mibs(vmm_freemem + vmm_usedmem), vmm_count_allocs()
	);
	printk(
		"PMM: %lld KiB free; %lld KiB used; %lld MiB total\n",
		bytes2kibs(pmm_freemem), bytes2kibs(pmm_usedmem), bytes2mibs(pmm_freemem + pmm_usedmem)
	);
	printk(
		"PMM4G: %lld KiB free; %lld KiB used; %lld MiB total\n",
		bytes2kibs(pmm4g_freemem), bytes2kibs(pmm4g_usedmem), bytes2mibs(pmm4g_freemem + pmm4g_usedmem)
	);

	khang();
}

noret void khang() {
	while (1) {
		arch_halt();
	}
}
