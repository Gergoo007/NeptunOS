#include <fs/vfs.hh>
#include <devmgr/msd/msd.hh>

vfs_entry* vfs_lookup(const filesystem& p, const char* path) {
	vfs_handle* h = (vfs_handle*)p.fshandle;
	vfs_entry* e = h->root;
	while (*path) {
		if (*path == '/') path++;

		// Megvan a keresett mappa/file?
		if (*path == 0)
			return e;

		// Milyen hosszú a jelenlegi szegmens?
		u32 namelen = 0;
		while (path[namelen] != '/' && path[namelen] != 0) namelen++;

		auto& hashmap = e->content.get<vfs_dir>();
		if (!hashmap.has(string(path, namelen)))
			return nullptr;

		e = &hashmap[string(path, namelen)];

		path += namelen;
	}
	return e;
}

u64 vfs_read(const filesystem& p, const char* path, u64 offset, u64 bytes, void* buf) {
	vfs_entry* file = vfs_lookup(p, path);
	if (file->content.active == 0) {
		error("Attempted to read a folder!");
		return -1;
	} else {
		auto& content = file->content.get<vector<u8>>();
		if (buf && content.data && bytes) {
			if (offset > content.size) {
				error("Offset '%llx' is bigger than the file (%llx)!", offset, content.size);
				return -1;
			}

			if (offset + bytes > content.size) {
				error("Offset + read size '%llx + %llx' is bigger than the file (%llx)!", offset, bytes, content.size);
				return -1;
			}

			u64 toread = min(bytes, content.size);
			memcpy(buf, file->content.get<vector<u8>>().data + offset, toread);
			return toread;
		} else {
			return 0;
		}
	}
}

u64 vfs_write(const filesystem& p, const char* path, u64 offset, u64 bytes, void* buf) {
	vfs_entry* file = vfs_lookup(p, path);
	if (file->content.active == 0) {
		error("Attempted to write to a folder!");
		return -1;
	} else {
		auto& content = file->content.get<vector<u8>>();
		if (buf && bytes) {
			// Ha nem elég nagy a file, meg kell növelni a méretét
			if (content.size < offset + bytes)
				content.resize(offset + bytes);
			memcpy(content.data + offset, buf, bytes);
			return bytes;
		} else {
			return 0;
		}
	}
}

void vfs_remove(const filesystem& p, const char* path) {
	const char* basename = path_basename((char*)path);

	vfs_entry* mappa = vfs_lookup(p, string(path, (u64)basename - (u64)path).c_str());
	auto& m = mappa->content.get<vfs_dir>();

	if (!m.has(basename)) {
		error("File or folder to be deleted doesn't exist!");
		return;
	}

	m.remove(basename);
}

void vfs_create(const filesystem& p, const char* path, bool mkdir) {
	const char* basename = path_basename((char*)path);

	vfs_entry* mappa = vfs_lookup(p, string(path, (u64)basename - (u64)path).c_str());
	if (mkdir)
		mappa->content.get<vfs_dir>()[basename] = vfs_entry(basename, vfs_entry_union(true, vfs_dir()));
	else
		mappa->content.get<vfs_dir>()[basename] = vfs_entry(basename, vfs_entry_union(true, vector<u8>()));
}

void vfs_mount(filesystem &f) {
	vfs_handle* h = new vfs_handle;
	f.fshandle = h;
	h->root = new vfs_entry;
	h->root->name = "";
	h->root->content.emplace_back<vfs_dir>();

	f.calls = vfs_calls;
}

vector<fs_entry> vfs_readdir(const filesystem& f, const char* path) {
	auto* e = vfs_lookup(f, path);
	vector<fs_entry> ret;
	for (const auto& file : e->content.get<vfs_dir>())
		ret.push_back(fs_entry { file.value.name, (bool)file.value.content.active });
	return ret;
}

optional<fs_entry> vfs_readmeta(const filesystem& f, const char* path) {
	auto* e = vfs_lookup(f, path);
	if (e)
		return optional(fs_entry { .name = e->name, .dir = (bool)e->content.active });
	else
		return optional<fs_entry>();
}
