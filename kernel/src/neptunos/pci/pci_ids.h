#pragma once

#include <neptunos/libk/stdall.h>
#include <neptunos/graphics/text_renderer.h>

#define KNOWN_MANUFACTURERS 2
#define KNOWN_PRODUCTS 2

typedef struct pci_id_pair {
	uint16_t id;
	char* name;
} pci_id_pair;

extern const pci_id_pair pci_vendors[];
extern const pci_id_pair pci_products[];

const char* pci_find_vendor(uint16_t id);
const char* pci_find_product(uint16_t id);

const char* pci_find_class(uint8_t class);
const char* pci_find_subclass(uint8_t class, uint8_t subclass);