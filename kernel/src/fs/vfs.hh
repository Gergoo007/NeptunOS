#pragma once

#include <fs/fs.hh>

struct vfs_entry;
using vfs_dir = hashmap<string, vfs_entry>;

using vfs_entry_union = variant<vfs_dir, vector<u8>>;
struct vfs_entry {
	string name;
	vfs_entry_union content;
};

struct vfs_handle {
	vfs_entry* root;
};

vfs_entry* vfs_lookup(const filesystem& p, const char* path);
u64 vfs_read(const filesystem& f, const char* path, u64 offset, u64 bytes, void* buf);
vector<fs_entry> vfs_readdir(const filesystem& f, const char* path);
u64 vfs_write(const filesystem& f, const char* path, u64 offset, u64 bytes, void* buf);
void vfs_remove(const filesystem& f, const char* path);
void vfs_create(const filesystem& f, const char* path, bool mkdir);
optional<fs_entry> vfs_readmeta(const filesystem& f, const char* path);

void vfs_mount(const filesystem& f);

constexpr fs_calltable vfs_calls {
	.read = vfs_read,
	.write = vfs_write,
	.remove = vfs_remove,
	.create = vfs_create,
	.readdir = vfs_readdir,
	.readmeta = vfs_readmeta,
};
