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
#include <cppcompat.hh>
#include <devmgr/input/input.hh>
#include <devmgr/usb/usb.hh>
#include <scheduler/scheduler.hh>
#include <fs/fs.hh>
#include <loader/loader.hh>
#include <arch/amd64/syscalls/syscalls.hh>
#include <arch/amd64/apic.hh>

extern "C" attr_noret void khang();
void test();

// For GDB debugging
bool watch = false;

// TODO: !!FONTOS!! A KERNEL LEGYEN 2 GIB-EN BELÜL, EZÁLTAL LEHESSEN HASZNÁLNI AZ -mcmodel=kernel-T, MERT MOST TELE VAN MINDEN MOVABS-AL
// TODO: ahci vezérlő && port reset
// TODO: device_t::subsys redundáns mivel a variant::active ugyanezt mutatja (már kurvára nincs kedvem revampolni, pedig ide az kell)
// TODO: msd_read optimalizáció
// TODO: WATERMARK LECSERÉLÉSE A map_page-NÉL

// !! VMM CHECKING DOES NOT WORK ON BOCHS BECAUSE IT IS TOO SLOW (BOCHS) !!

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

	input_init();

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
		bytes2kibs(g_vmm.freemem), bytes2kibs(g_vmm.usedmem), bytes2mibs(g_vmm.freemem + g_vmm.usedmem), g_vmm.count_allocs()
	);
	printk(
		"PMM: %lld KiB free; %lld KiB used; %lld MiB total\n",
		bytes2kibs(pmm_freemem), bytes2kibs(pmm_usedmem), bytes2mibs(pmm_freemem + pmm_usedmem)
	);
	printk(
		"PMM4G: %lld KiB free; %lld KiB used; %lld MiB total\n",
		bytes2kibs(g_pmm4g.freemem), bytes2kibs(g_pmm4g.usedmem), bytes2mibs(g_pmm4g.freemem + g_pmm4g.usedmem)
	);

	init_syscalls();

	// Dry read; ezzel lehet megszerezni a fájlméretet a legegyszerűbben
	// vector<fs_entry> root = fs_readdir("/");
	// report("sysc 3");
	// for (const auto& f : root) {
	// 	report("/%s: %s", f.name.c_str(), f.dir ? "folder" : "file");
	// 	if (f.dir) {
	// 		string path = path_join("/", f.name.c_str());
	// 		report("readin %s", path.c_str());
	// 		vector<fs_entry> dir = fs_readdir(path.c_str());
	// 		for (const auto& f2 : dir) {
	// 			report("%s: %s", path_join(path.c_str(), f2.name.c_str()).c_str(), f.dir ? "folder" : "file");
	// 		}
	// 	}
	// }

	khang();
}

attr_noret void khang() { while (1) arch_halt(); }
