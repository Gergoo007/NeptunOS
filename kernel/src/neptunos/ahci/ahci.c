#include "ahci.h"
#include <neptunos/graphics/text_renderer.h>

hba_mem* abar = NULL;

ahci_port_type ahci_check_port_type(hba_port* port) {
	sata_status_t status = *((sata_status_t*) &port->sata_status);

	// Device cannot be communicated with
	if (status.det != DET_PRESENT_PHY) return AHCI_NONE;
	// Device not powered on
	if (status.ipm != IPM_ACTIVE) return AHCI_NONE;
	// Device cannot be communicated with
	if (status.spd == SPD_NOT_DETECTED) return AHCI_NONE;

	switch (port->signature) {
		case SATA_SIG_ATAPI:
			return AHCI_SATAPI;

		case SATA_SIG_ATA:
			return AHCI_SATA;

		case SATA_SIG_PM:
			return AHCI_PM;

		case SATA_SIG_SEMB:
			return AHCI_SEMB;

		case SATA_SIG_NONE:
			return AHCI_NONE;

		default:
			printk("\nUnknown signature: %04x\n", port->signature);
			return AHCI_NONE;
	}
}

void probe_ports() {
	if (abar == NULL)
		printk("Hey! What are you doing?\n");
	else
		printk("It's fine: %p\n", abar);
	
	uint32_t ports_impl = abar->ports_impl;
	for (uint8_t i = 0; i < 32; i++) {
		if (ports_impl & (1 << i)) {
			ahci_port_type port_type = ahci_check_port_type(&abar->ports[i]);

			switch (port_type) {
				case AHCI_SATA:
					printk("SATA device detected!\n");
					break;
				case AHCI_SATAPI:
					printk("SATAPI device detected!\n");					
					break;
				case AHCI_PM:
					printk("Port muliplier device detected!\n");
					break;
				case AHCI_SEMB:
					printk("SEMB device detected!\n");
					break;
				case AHCI_NONE:
					break;
				default:
					printk("BUG: port_type out of bounds! (%d)\n", port_type);
					break;
			}
		}
	}
}

void ahci_init(pci_device_header_0_t* device) {
	printk("\nAHCI init called\n");
	printk("Device: %04x:%04x > %s %s\n", device->base.vendor_id, device->base.device_id, 
		pci_find_class(device->base.class), 
		pci_find_subclass(device->base.class, device->base.subclass));

	abar = (hba_mem*)((uint64_t)device->base_addr_5);
	map_address(abar, abar);

	probe_ports();
}
