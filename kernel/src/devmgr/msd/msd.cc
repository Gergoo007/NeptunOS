#include <devmgr/msd/msd.hh>
#include <devmgr/devmgr.hh>
#include <util/mem.hh>
#include <util/printf.hh>
#include <util/guid.hh>

void msd_read(device_t& d, u64 offset, u64 bytes, void* buf) {
	auto* funcs = (msd_call_table*)d.extra;
	if (isaligned(offset, 512) && isaligned(bytes, 512) && isaligned(buf, 512)) {
		funcs->read(d, offset / 512, bytes / 512, buf);
	} else {
		void* bounce = kmalloc_aligned(align(bytes, 512), 512);
		funcs->read(d, offset / 512, align(bytes, 512) / 512, bounce);
		memcpy(buf, (void*)((u64)bounce + (offset % 512)), bytes);
		kfree(bounce);
	}
}

PartitionType guid2parttype(u8* guid) {
	switch (*(u128*)guid) {
		case guid_parse::make_guid("C12A7328-F81F-11D2-BA4B-00A0C93EC93B").getraw(): return PartitionType::EFI_SYSTEM_PARTITION;
		case guid_parse::make_guid("0FC63DAF-8483-4772-8E79-3D69D8477DE4").getraw(): return PartitionType::LINUX_FILESYSTEM;
		case guid_parse::make_guid("EBD0A0A2-B9E5-4433-87C0-68B6B72699C7").getraw(): return PartitionType::MICROSOFT_BASIC_DATA;
		case guid_parse::make_guid("E3C9E316-0B5C-4DB8-817D-F92DF00215AE").getraw(): return PartitionType::MICROSOFT_RESERVED;
		default: fatal("he");
	}
}

// Read the partition table and register its entries
void msd_scout(device_t& d) {
	assert(d.subsys == DevmgrSubsys::MSD);

	gpt* hdr = (gpt*)kmalloc_aligned(512, 512);
	gpt_entry* ents = (gpt_entry*)kmalloc_aligned(512, 512);
	msd_read(d, 512, 512, hdr);

	auto& msd = d.kinds.get<device_t_MSD>();

	if (memcmp((void*)hdr, (void*)"EFI PART", 8)) {
		warn("Disk is not GPT!");
		goto ret;
	}

	warn("start: %lld, first: %lld, end: %lld", hdr->startlba, hdr->firstlba, hdr->lastlba);
	warn("gpt guid: %s", guid2string(hdr->guid).c_str());

	for (u32 i = 0; i < hdr->num_parts; i++) {
		// Új szektorba léptünk
		if (i % 4 == 0)
			msd_read(d, (hdr->startlba + align(i, 4) / 4) * 512, 512, ents);

		gpt_entry& e = ents[i % 4];
		if (*(u128*)e.type == 0) continue;
		report("part guid: %s", guid2string(e.type).c_str());
		guid2parttype(e.type);

		partition* p = &msd.parts.emplace_back(partition {
			.type = PartitionType::OTHER,
			.offset = e.start * 512,
			.size = ((e.end + 1) - e.start) * 512,
			.name = e.name,
			.parent = d,
		});

		// Mount megkísérlése
		fs_mount(p, "/", FilesystemType::CUSTOM);
		// auto files = fs_readdir("/");
		// for (const auto& f : files) {
		// 	report("found file of type %d: %s", f.dir, f.name.c_str());
		// }
	}

ret:
	kfree(hdr);
	kfree(ents);
}
