#include <types.hh>
#include <devmgr/module.hh>
#include <devmgr/devmgr.hh>
#include <devmgr/msd/msd.hh>

#include "fat.hh"

__attribute__((section(".modinfo"), used))
volatile constexpr module_metadata_t _modinfo {
	.name = "fat",
	.triggertype = ModuleTriggerTypes::FILESYSTEM,
	.trigger = { .FILESYSTEM {  } }
};

struct fat_fshandle {
	fat_bpb* bpb;
	fat_extbootrec* ebr;
	fat_fsinfo* fsi;
};

extern "C" bool mod_main(filesystem& f) {
	if (f.fshandle) fatal("Filesystem is already scanned!");

	// device_t& d = p.parent;

	// p.fshandle = kmalloc(sizeof(fat_fshandle));
	// fat_fshandle* h = (fat_fshandle*)p.fshandle;
	
	// u64 alloc = (u64)kmalloc_aligned(512 * 3, 512);
	// h->bpb = (fat_bpb*)alloc;
	// h->ebr = (fat_extbootrec*)(alloc + 512);
	// h->fsi = (fat_fsinfo*)(alloc + 1024);

	// msd_read(d, p.offset, 512, h->bpb);
	

	return true;
}
