#include <storage/interface.h>

stg_dev device_parse(const char* s) {
	stg_dev ret;
	u8 fivechar = 0;
	if (!strncmp(s, "msd", 3)) {
		ret.type = STG_MSD;
	} else if (!strncmp(s, "atapi", 5)) {
		fivechar = 1;
		ret.type = STG_ATAPI;
	} else if (!strncmp(s, "ata", 3)) {
		ret.type = STG_ATA;
	} else {
		ret.type = -1;
	}

	ret.num = str_to_uint(s + (fivechar ? 5 : 3));

	return ret;
}
