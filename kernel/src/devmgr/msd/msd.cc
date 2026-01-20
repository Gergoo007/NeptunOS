#include <devmgr/msd/msd.hh>
#include <devmgr/devmgr.hh>

void msd_read(device_t& d, u64 offset, u64 bytes, void* buf) {
	auto* funcs = (msd_call_table*)d.extra;
	if (aligned(offset, 512) && aligned(bytes, 512) && aligned(buf, 512)) {
		funcs->read(d, offset / 512, bytes / 512, buf);
	} else {
		void* bounce = kmalloc_aligned(align(bytes, 512), 512);
		funcs->read(d, offset / 512, align(bytes, 512) / 512, bounce);
		memcpy(buf, (void*)((u64)bounce + (offset % 512)), bytes);
		kfree(bounce);
	}
}

// Read the partition table and register its entries
void msd_scout(device_t& d) {
	assert(d.subsys == DevmgrSubsys::MSD);

	char* o = (char*)kmalloc(512);

	msd_read(d, 0, 10, o);
	report("o1 is %s", o);
	msd_read(d, 512, 10, o);
	report("o2 is %s", o);

	kfree(o);
}
