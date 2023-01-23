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

char* pci_find_vendor(uint16_t id);
char* pci_find_product(uint16_t id);
