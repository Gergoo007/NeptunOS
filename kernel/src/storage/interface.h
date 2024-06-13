#pragma once

#include <lib/types.h>
#include <lib/string.h>

enum {
	STG_MSD = 0x00,
	STG_ATA = 0x01,
	STG_ATAPI = 0x02,
};

typedef struct {
	u8 num;
	u8 type;
} stg_dev;

stg_dev device_parse(const char* s);
