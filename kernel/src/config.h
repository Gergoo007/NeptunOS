#pragma once

#include <lib/attrs.h>

// Define this if you want printk to log into the
// serial port and the console at the same time
#define PRINTK_SERIAL

_attr_unused static char* _logs[] = {
	"ahci.c",
	"pci.c",
};
