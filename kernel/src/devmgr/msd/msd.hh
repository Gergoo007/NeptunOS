#pragma once

#include <types.hh>

struct device_t;

struct partition {
	
};

struct msd_call_table {
	void (*read)(device_t& msd, u64 lba, u64 sectors, void* buf);
};

void msd_scout(device_t& drive);

void msd_read(device_t& d, u64 offset, u64 bytes, void* buf);
