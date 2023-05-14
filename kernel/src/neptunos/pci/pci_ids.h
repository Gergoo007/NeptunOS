#pragma once

#include <neptunos/libk/stdall.h>
#include <neptunos/graphics/text_renderer.h>

#define PCI_CLASS_MSD 0x01
#define PCI_CLASS_NET 0x02
#define PCI_CLASS_DIS 0x03
#define PCI_CLASS_MM  0x04
#define PCI_CLASS_MEM 0x05
#define PCI_CLASS_BRI 0x06

#define PCI_SCLASS_IDE  0x01
#define PCI_SCLASS_SATA 0x06

#define PCI_PI_VEND_SPEC 0x00
#define PCI_PI_AHCI_1_0	 0x01
#define PCI_PI_SER_STOR_BUS 0x02

typedef struct pci_id_pair {
	uint16_t id;
	const char* name;
} pci_id_pair;

extern const pci_id_pair pci_vendors[];
extern const pci_id_pair pci_products[];

const char* pci_find_vendor(uint16_t id);
const char* pci_find_product(uint16_t id);

const char* pci_find_class(uint8_t class);
const char* pci_find_subclass(uint8_t class, uint8_t subclass);