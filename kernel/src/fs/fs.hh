#pragma once

#include <util/storage.hh>
#include <util/variant.hh>

struct fs_entry {
	string name;
	bool dir;
};

struct filesystem;
struct fs_calltable {
	u64 (*read)(const filesystem& f, const char* path, u64 offset, u64 bytes, void* data) = nullptr;
	u64 (*write)(const filesystem& f, const char* path, u64 offset, u64 bytes, void* data) = nullptr;
	void (*remove)(const filesystem& f, const char* path) = nullptr;
	void (*create)(const filesystem& f, const char* path, bool mkdir) = nullptr;
	vector<fs_entry> (*readdir)(const filesystem& f, const char* path) = nullptr;
	optional<fs_entry> (*readmeta)(const filesystem& f, const char* path);
};

enum struct FilesystemType : u32 {
	CUSTOM, // Ami modulban rejtőzik, pl. fat32, majd ext4 stb.
	VFS,
};

struct partition;
struct filesystem {
	partition* p;
	fs_calltable calls;
	void* fshandle;
	FilesystemType type;
};

struct mountpoint {
	filesystem f;
	string path;
};

void fs_mount(partition* p, const char* path, FilesystemType fstype);
const mountpoint* fs_find_mnt(const char* p);
string path_sanitize(const char* p);
const char* path_basename(char* p);
void fs_create(const char* path, bool mkdir);
vector<fs_entry> fs_readdir(const char* path);
u64 fs_write(const char* path, u64 offset, u64 bytes, void* buf);
u64 fs_read(const char* path, u64 offset, u64 bytes, void* buf);
