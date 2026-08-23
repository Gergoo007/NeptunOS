#include <types.hh>
#include <devmgr/module.hh>
#include <devmgr/devmgr.hh>
#include <devmgr/msd/msd.hh>
#include <util/string.hh>
#include <mm/vmm.hh>
#include <util/string.hh>

#include "fat.hh"

__attribute__((section(".modinfo"), used))
volatile constexpr ModuleMetadata _modinfo {
	.name = "fat",
	.triggertype = ModuleTriggerTypes::FILESYSTEM,
	.trigger = { .FILESYSTEM {  } }
};

struct fat_fshandle {
	fat_bpb* bpb;
	fat_fsinfo* fsi;
	u32* fat;
	u64 fatsize;
	u64 dataoffset; // Root directory offset byte-okban
};

u64 clusteroff(const filesystem& f, u32 clusterlo, u32 clusterhi) {
	fat_fshandle* h = (fat_fshandle*)f.fshandle;
	return h->dataoffset + ((clusterlo | ((clusterhi) << 16)) - 2) * h->bpb->bytespersector * h->bpb->sectorspercluster;
}

// Külön function kell még erre is mert az egész egy legacy kupac szar
Opt<String> fat32_getname(const char* longname, fat_entry* en) {
	if (longname[0]) {
		return longname;
	} else {
		if (!strncmp((char*)en->simplename, ".", 1)) return {};
		if (!strncmp((char*)en->simplename, "..", 2)) return {};
		if (en->simplename[0] == ' ') return {};
		// A fájl előzőleg ki lett törölve
		if (en->simplename[0] > 128) return {};

		u32 len = 0;
		while (en->simplename[len] != ' ') len++;

		if (!len) return {};

		return String((char*)en->simplename, len);
	}
}

Opt<fat_entry> fat32_lookup(const filesystem& f, const char* path) {
	fat_fshandle* h = (fat_fshandle*)f.fshandle;
	unique_ptr<fat_entry> ent = (fat_entry*)kmalloc_aligned(512, 512);
	unique_ptr<char> longname = (char*)kmalloc(256);
	memset(longname, 0, 256);
	msd_read(f.p->parent, h->dataoffset, 512, ent);

	// Már megint valami agyhalott kitalálta hogy a cluster{lo,hi} mezők a root directory-nél
	// ne legyenek érvényesek
	ent->cluster_hi16 = 0;
	ent->cluster_lo16 = 2;

	while (*path) {
		while (*path == '/') path++;

		if (*path == 0) return *ent;

		u32 seglen = 0;
		while (path[seglen] != '/' && path[seglen] != 0) seglen++;

		String basename = String(path, seglen);

		bool found = false;
		for (u64 i = 0; ent[i].attrs && i < 16; i++) {
			fat_lfn_entry* lfn = (fat_lfn_entry*)&ent[i];
			if (ent[i].attrs == FatAttrs::LFN) {
				ucs2_to_asciin(lfn->no1, (char*)longname + (lfn->order - 1) * 13, 5);
				ucs2_to_asciin(lfn->no2, (char*)longname + (lfn->order - 1) * 13 + 5, 6);
				ucs2_to_asciin(lfn->no3, (char*)longname + (lfn->order - 1) * 13 + 11, 2);
			} else {
				if (ent[i].attrs & FatAttrs::VOLUME_ID) continue;

				// if (longname[0])
				// 	report("Found %s called %s (%.11s)", ent[i].attrs & FatAttrs::DIRECTORY ? "folder" : "file", longname, ent->simplename);
				// else
				// 	report("Found %s called %.11s", ent[i].attrs & FatAttrs::DIRECTORY ? "folder" : "file", ent->simplename);

				auto name = fat32_getname(longname, ent + i);
				if (name.present) {
					if (!strcmp(basename.c_str(), name.expect("???").c_str())) {
						path += seglen;
						if (*path == 0) return ent[i];
						msd_read(f.p->parent, clusteroff(f, ent[i].cluster_lo16, ent[i].cluster_hi16), 512, ent);
						found = true;
						longname[0] = 0;
					}
				}
			}
		}

		if (!found)
			fatal("No such file or folder: %s", basename.c_str());
	}

	fatal("File not found!");
}

static constexpr u32 clusterfrom(u32 lo, u32 hi) { return lo | (hi << 16); }

static u64 diskoffset(const filesystem& f, u32 cluster) {
	auto* h = ((fat_fshandle*)f.fshandle);
	assert(cluster >= 2);
	return h->dataoffset + (cluster - 2) * 512;
}

// Beolvas byte-nyi byte-ot offset-ről buf-ba, használva a FAT-ot
// Ha a buf nullptr, akkor csak a fájlméretet adja vissza, ez haszos a mappáknál ahol a fat_entry::size érvénytelen mert valami idióta ezt találta ki
static u64 chainread(const filesystem& f, u32 cluster, u64 offset, u64 bytes, void* buf, u32 fsize) {
	fat_fshandle* h = (fat_fshandle*)f.fshandle;
	u64 bytespercluster = h->bpb->bytespersector * h->bpb->sectorspercluster;
	if (!buf) bytes = -1u;

	// Offszetnyi cluster-t át kell skippelni
	for (u32 i = 0; i < offset / bytespercluster; i++) {
		u32 fatent = h->fat[cluster] & 0x0fffffff;
		if (cluster >= 0x0ffffff8) fatal("File not big enough to skip to offset?? offset: %lld; file size: %d", offset, fsize);
		cluster = fatent;
	}

	// A maradék offszetet már lepasszolhatom az msd_read-nek
	u64 clusteroff = offset % bytespercluster;

	// Ennyi clustert kell külön beolvasni
	u64 bytes_read = 0;
	while (bytes) {
		u64 readlen = min(bytes, bytespercluster - clusteroff);

		if (buf)
			msd_read(f.p->parent, diskoffset(f, cluster) + clusteroff, readlen, buf);
		clusteroff = 0;
		bytes_read += readlen;

		u32 fatent = h->fat[cluster] & 0x0fffffff;
		if (fatent >= 0x0ffffff8) {
			if (bytes - readlen) {
				if (buf)
					warn("Premature end of file: %lld remaining (%lld read just now)", bytes - readlen, readlen);
			}
			return bytes_read;
		}
		cluster = fatent;

		if (buf)
			bytes -= readlen;
		if (buf)
			buf = (void*)((u64)buf + readlen);
	}
	return bytes_read;
}

u64 fat32_read(const filesystem& f, const char* path, u64 offset, u64 bytes, void* buf) {
	fat_entry e = fat32_lookup(f, path).expect("File not found!");
	if (!buf) {
		assert((e.attrs & FatAttrs::DIRECTORY) == 0);
		return e.size;
	}

	u64 cluster = clusterfrom(e.cluster_lo16, e.cluster_hi16);
	assert(offset <= e.size);
	return chainread(f, cluster, offset, bytes, buf, e.size);
}

Vector<fs_entry> fat32_readdir(const filesystem& f, const char* path) {
	report("clusters: %d %s: %d", fat32_lookup(f, "/").expect("Folder not found!").cluster_lo16, path, fat32_lookup(f, path).expect("Folder not found!").cluster_lo16);

	fat_entry e = fat32_lookup(f, path).expect("Folder not found!");
	Vector<fs_entry> entries;
	u32 cluster = clusterfrom(e.cluster_lo16, e.cluster_hi16);
	u64 bytes = chainread(f, cluster, 0, -1ull, nullptr, -1u);
	entries.reserve(bytes / sizeof(fat_entry));
	fat_entry* tmp = (fat_entry*)kmalloc(bytes);
	chainread(f, cluster, 0, bytes, tmp, -1u);

	fat_entry* en = tmp;
	unique_ptr<char> longname = (char*)kmalloc(256);
	longname[0] = 0;
	while (en->attrs) {
		fat_lfn_entry* lfn = (fat_lfn_entry*)en;
		if (en->attrs == FatAttrs::LFN) {
			ucs2_to_asciin(lfn->no1, (char*)longname + (lfn->order - 1) * 13, 5);
			ucs2_to_asciin(lfn->no2, (char*)longname + (lfn->order - 1) * 13 + 5, 6);
			ucs2_to_asciin(lfn->no3, (char*)longname + (lfn->order - 1) * 13 + 11, 2);
		} else {
			if (en->attrs & FatAttrs::VOLUME_ID) goto cont;

			if (longname[0]) {
				entries.emplace_back(fs_entry {
					.name = (char*)longname,
					.dir = (en->attrs & FatAttrs::DIRECTORY) > 0,
				});

				longname[0] = 0;
			} else {
				auto name = fat32_getname(longname, en);
				if (name.present) {
					entries.emplace_back(fs_entry {
						.name = name.expect("??"),
						.dir = (en->attrs & FatAttrs::DIRECTORY) > 0,
					});
				}
				longname[0] = 0;
			}
		}

cont:
		en++;
	}
	
	return entries;
}

u64 fat32_write(const filesystem& f, const char* path, u64 offset, u64 bytes, void* buf) {
	fatal("Not implemented!");
}

void fat32_remove(const filesystem& f, const char* path) {
	fatal("Not implemented!");
}

void fat32_create(const filesystem& f, const char* path, bool mkdir) {
	fatal("Not implemented!");
}

Opt<fs_entry> fat32_readmeta(const filesystem& f, const char* path) {
	fatal("yes");
}

extern "C" bool mod_main(filesystem& f) {
	if (f.fshandle) fatal("Filesystem is already scanned!");

	Device& d = f.p->parent;

	f.fshandle = kmalloc(sizeof(fat_fshandle));
	fat_fshandle* h = (fat_fshandle*)f.fshandle;

	u64 alloc = (u64)kmalloc_aligned(512 * 2, 512);
	h->bpb = (fat_bpb*)alloc;
	h->fsi = (fat_fsinfo*)(alloc + 512);

	if (!f.p->offset)
		fatal("FAT volume at the start of the drive?");
	msd_read(d, f.p->offset, 512, h->bpb);

	u32 bps = h->bpb->bytespersector;

	if (bps < 512 || bps > 0x1000) {
		error("FAT BPS is %d, exiting...", bps);
		return false;
	}

	if (!h->bpb->sectorspercluster) {
		error("FAT sectorspercluster is 0, exiting...");
		return false;
	}

	if (h->bpb->num_fats != 1 && h->bpb->num_fats != 2) {
		error("FAT num_fats is %d, exiting...", h->bpb->num_fats);
		return false;
	}

	debug("fat bytes/sector %d", h->bpb->bytespersector);
	debug("fat sectors/cluster %d", h->bpb->sectorspercluster);
	debug("fat num sectors %d/%d", h->bpb->num_sectors, h->bpb->num_sectors_ext);
	debug("fat sectors/fat %d", h->bpb->sectorsperfat);
	debug("fat num fats %d", h->bpb->num_fats);
	debug("fat num root dir ents %d", h->bpb->num_root_dir_ents);
	debug("fat num hidden sectors %d", h->bpb->num_hidden_sectors);

	debug("EXT sectors/fat %d", h->bpb->ext.sectorsperfat);
	debug("EXT rootdir cluster %d", h->bpb->ext.rootdir_cluster);
	debug("EXT fatver %d", h->bpb->ext.fatver);

	debug("bytes/cluster: %d", h->bpb->sectorspercluster * bps);

	h->fatsize = h->bpb->ext.sectorsperfat * bps;
	h->fat = (u32*)kmalloc(h->fatsize);
	u64 fatoffset = f.p->offset + h->bpb->num_reserved_sectors * bps;
	msd_read(d, fatoffset, h->fatsize, h->fat);

	h->dataoffset = fatoffset + (u64)h->bpb->num_fats * h->bpb->ext.sectorsperfat * bps;

	// A FAT32 mount sikeres volt, ki kell írni a modul function-jeit
	f.calls = fs_calltable {
		.read = fat32_read,
		.write = fat32_write,
		.remove = fat32_remove,
		.create = fat32_create,
		.readdir = fat32_readdir,
		.readmeta = fat32_readmeta,
	};

	return true;
}
