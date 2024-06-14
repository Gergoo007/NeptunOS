#pragma once

#include <pci/pci.h>

#include <lib/string.h>

typedef struct pci_pair {
	u16 id;
	char* name;
} pci_pair;

_attr_unused
static pci_pair vendors[] = {
	{ 0x8086, "Intel" },
	{ 0x80ee, "VirtualBox" },
	{ 0x1234, "Bochs" },
	{ 0x1002, "AMD" },
	{ 0x12d2, "NVidia" },
	{ 0x1b21, "ASMedia" },
	{ 0x1849, "ASRock" },
	{ 0x17aa, "Lenovo" },
	{ 0x17ef, "Lenovo" },
};

_attr_unused
static char* classes[] = {
	"Unclassified",
	"Mass Storage Controller",
	"Network Controller",
	"Display Controller",
	"Multimedia Controller",
	"Memory Controller",
	"Bridge",
	"Simple Communication Controller",
	"Base System Peripheral",
	"Input Device Controller",
	"Docking Station",
	"Processor",
	"Serial Bus Controller",
	"Wireless Controller",
};

char* pci_vendor(u16 id);
char* pci_class(u8 class);
