#include <fs/fs.hh>
#include <devmgr/devmgr.hh>

Vector<mountpoint> mountpts;

String path_sanitize(const char* p) {
	String ret;
	u64 len = strlen(p);
	ret.reserve(len);
	ret.size = 0;

	while (*p) {
		ret += *p;

		if (*p == '/')
			while (*p == '/') p++;
		else
			p++;
	}
	return ret;
}

const char* path_basename(char* path) {
	char* basename = path + strlen(path);
	basename--;
	if (*basename == '/') {
		*basename = 0;
		basename--;
	}
	while (*basename != '/' && basename > path) basename--;
	if (*basename == '/')
		basename++;

	u32 len = 0;
	while (basename[len] != 0 && basename[len] != '/')
		len++;
	if (basename[len] == '/')
		basename[len] = 0;

	return basename;
}

String path_join(const char* path1, const char* path2) {
	String ret = path1;
	if (ret[ret.size - 1] != '/')
		ret += '/';
	ret += path2;
	return ret;
}

const mountpoint* fs_find_mnt(const char* p) {
	String san = path_sanitize(p);

	// Legközelebbi mountpoint megkeresése
	assert(mountpts.size);
	const mountpoint* closest = &mountpts[0];
	for (const auto& m : mountpts) {
		// Ha a mountpoint hosszabb mint a path akkor ki van zárva
		if (m.path.size > san.size) continue;

		u32 len = 0;
		const char* turi = m.path.c_str();
		while (*turi)
			(turi++, len++);

		if (!strncmp(p, san.c_str(), len))
			// A mountpoint substring-je a path-nek
			if (closest->path.size < m.path.size)
				closest = &m;
	}

	return closest;
}

// TODO: létezik ez a path (mnt) egyáltalán?
void fs_mount(Partition* p, const char* mnt, FilesystemType fstype) {
	String path = path_sanitize(mnt);
	mountpoint m { filesystem {
		.p = nullptr,
		.calls = {},
		.fshandle = nullptr,
		.type = fstype,
	}, path };
	if (fstype == FilesystemType::CUSTOM) {
		m.f.p = p;
		devmgr_try_mount(m.f);
	} else {
		extern void vfs_mount(filesystem &f);
		vfs_mount(m.f);
	}
	mountpts.emplace_back(mountpoint { m.f, path });
}

void fs_create(const char* path, bool mkdir) {
	const mountpoint* m = fs_find_mnt(path);
	const char* path2 = path + m->path.size;
	if (*path2 == 0)
		path2 = "/";
	if (m->f.calls.readmeta(m->f, path2).present)
		fatal("File already exists!");
	m->f.calls.create(m->f, path2, mkdir);
}

Opt<fs_entry> fs_readmeta(const char* p) {
	const mountpoint* asd = fs_find_mnt(p);
	const char* path2 = p + asd->path.size;
	if (*path2 == 0)
		path2 = "/";

	return asd->f.calls.readmeta(asd->f, path2);
}

Vector<fs_entry> fs_readdir(const char* p) {
	const mountpoint* asd = fs_find_mnt(p);
	const char* path2 = p + asd->path.size;
	if (*path2 == 0)
		path2 = "/";

	return asd->f.calls.readdir(asd->f, path2);
}

u64 fs_write(const char* p, u64 offset, u64 bytes, void* buf) {
	const mountpoint* asd = fs_find_mnt(p);
	const char* path2 = p + asd->path.size;
	if (*path2 == 0)
		path2 = "/";

	return asd->f.calls.write(asd->f, path2, offset, bytes, buf);
}

u64 fs_read(const char* p, u64 offset, u64 bytes, void* buf) {
	const mountpoint* asd = fs_find_mnt(p);
	const char* path2 = p + asd->path.size;
	if (*path2 == 0)
		path2 = "/";

	return asd->f.calls.read(asd->f, path2, offset, bytes, buf);
}
