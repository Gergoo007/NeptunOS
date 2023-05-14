#include "pci_ids.h"

const pci_id_pair pci_vendors[] = {
	{ 0x8086, "Intel" },
	{ 0x1234, "Bochs" }
};

const pci_id_pair pci_products[] = {
	{ 0x29c0, "82G33/G31/P35/P31 Express DRAM Controller" },
	{ 0x1111, "Virtual graphics" },
	{ 0x2918, "82801IB (ICH9) LPC Interface Controller" },
	{ 0x2922, "82801IR/IO/IH (ICH9R/DO/DH) 6 port SATA Controller [AHCI mode]" },
	{ 0x2930, "82801I (ICH9 Family) SMBus Controller" }
};

// 2D array for classes & subclasses
const char pci_subclasses[23][12][256] = {
	// Unclassified
	{
		{ "Non-VGA-Compatible unclassified device" },
		{ "VGA-Compatible unclassified device" } 
	},
	
	// Mass storage device
	{
		{ "SCSI Bus controller" },
		{ "IDE controller" },
		{ "Floppy disk controller" },
		{ "IPI bus controller" },
		{ "RAID controller" },
		{ "ATA controller" },
		{ "Serial ATA (SATA) controller" },
		{ "Serial Attached SCSI controller" },
		{ "Non-volatile memory controller" },
		{ "Unknown storage controller" },
	},

	// Network Controller
	{
		{ "Ethernet controller" },
		{ "Token ring controller" },
		{ "FDDI controller" },
		{ "ATM controller" },
		{ "ISDN controller" },
		{ "WorldFip controller" },
		{ "PICMG 2.14 Multi Computing controller" },
		{ "Infiniband controller" },
		{ "Fabric controller" },
		{ "Unknown network controller" },
	},

	// Video
	{
		{ "VGA Compatible controller" },
		{ "XGA controller" },
		{ "3D controller" },
		{ "Unknown display controller" },
	},

	// Multimedia
	{
		{ "Multimedia video controller" },
		{ "Multimedia audio controller" },
		{ "Computer telephony device" },
		{ "Audio device" },
		{ "Unknown multimedia device" },
	},

	// Memory
	{
		{ "RAM controller" },
		{ "Flash controller" },
		{ "Unknown memory controller" },
	},

	// Bridge
	{
		{ "Host bridge" },
		{ "ISA bridge" },
		{ "EISA bridge" },
		{ "MCA bridge" },
		{ "PCI-to-PCI bridge" },
		{ "PCMCIA bridge" },
		{ "NuBus bridge" },
		{ "CardBus bridge" },
		{ "RACEway bridge" },
		{ "PCI-to-PCI bridge" },
		{ "Infiniband-to-PCI bridge" },
		{ "Unknown bridge" },
	},

	// Communications
	{
		{ "Serial controller" },
		{ "Parallel controller" },
		{ "Multiport serial controller" },
		{ "Modem" },
		{ "IEEE 488.1/2 controller" },
		{ "Smart Card controller" },
		{ "Unknown communications controller" },
	},

	// Basic peripherals
	{
		{ "PIC" },
		{ "DMA controller" },
		{ "Timer" },
		{ "RTC controller" },
		{ "PCI Hot-Plug controller" },
		{ "SD host controller" },
		{ "IOMMU" },
		{ "Unknown peripheral" },
	},

	// Input controller
	{
		{ "Keyboard controller" },
		{ "Digitizer pen" },
		{ "Mouse controller" },
		{ "Scanner controller" },
		{ "Gameport controller" },
		{ "Unknown input controller" },
	},

	// Docking station
	{
		{ "Generic docking station" },
		{ "Unknown docking station" }
	},

	// Processor
	{
		{ "386 processor" },
		{ "486 processor" },
		{ "Pentium processor" },
		{ "Pentium Pro processor" },
		{ "Alpha processor" },
		{ "PowerPC processor" },
		{ "MIPS processor" },
		{ "Co-Processor processor" },
		{ "Unknown processor" },
	},

	// Serial bus controller
	{
		{ "FireWire (IEEE 1394) controller" },
		{ "ACCESS Bus controller" },
		{ "SSA" },
		{ "USB controller" },
		{ "Fibre channel" },
		{ "SMBus controller" },
		{ "Infiniband controller" },
		{ "" }
	}
};

// 2D array for classes & subclasses
const char pci_program_interfaces[23][12][8][256] = {
	// Unclassified
	{
		{ 
			// Non-VGA-Compatible unclassified device
			{ "" },
			// VGA-Compatible unclassified device
			{ "" },
		},
	},
	
	// Mass storage device
	{
		// SCSI Bus controller
		{ "" },
		// IDE controller
		{ "" },
		// Floppy disk controller
		{ "" },
		// IPI bus controller
		{ "" },
		// RAID controller
		{ "" },
		// ATA controller
		{ "" },
		// Serial ATA (SATA) controller
		{ "Vendor specific interface", "AHCI 1.0", "Serial storage bus" },
		// Serial Attached SCSI controller
		{ "" },
		// Non-volatile memory controller
		{ "" },
		// Unknown storage controller
		{ "" },
	},

	// Network Controller
	{
		// Ethernet controller"
		{ "" },
		// Token ring controller"
		{ "" },
		// FDDI controller"
		{ "" },
		// ATM controller"
		{ "" },
		// ISDN controller"
		{ "" },
		// WorldFip controller"
		{ "" },
		// PICMG 2.14 Multi Computing controller"
		{ "" },
		// Infiniband controller"
		{ "" },
		// Fabric controller"
		{ "" },
		// Unknown network controller"
		{ "" },
	},

	// Video
	{
		// VGA Compatible controller"
		{ "" },
		// XGA controller"
		{ "" },
		// 3D controller"
		{ "" },
		// Unknown display controller"
		{ "" },
	},

	// Multimedia
	{
		// Multimedia video controller"
		{ "" },
		// Multimedia audio controller"
		{ "" },
		// Computer telephony device"
		{ "" },
		// Audio device"
		{ "" },
		// Unknown multimedia device"
		{ "" },
	},

	// Memory
	{
		// RAM controller"
		{ "" },
		// Flash controller"
		{ "" },
		// Unknown memory controller"
		{ "" },
	},

	// Bridge
	{
		// Host bridge
		{ "" },
		// ISA bridge
		{ "" },
		// EISA bridge
		{ "" },
		// MCA bridge
		{ "" },
		// PCI-to-PCI bridge
		{ "" },
		// PCMCIA bridge
		{ "" },
		// NuBus bridge
		{ "" },
		// CardBus bridge
		{ "" },
		// RACEway bridge
		{ "" },
		// PCI-to-PCI bridge
		{ "" },
		// Infiniband-to-PCI bridge
		{ "" },
		// Unknown bridge
		{ "" },
	},

	// Communications
	{
		// Serial controller"
		{ "" },
		// Parallel controller"
		{ "" },
		// Multiport serial controller"
		{ "" },
		// Modem"
		{ "" },
		// IEEE 488.1/2 controller"
		{ "" },
		// Smart Card controller"
		{ "" },
		// Unknown communications controller"
		{ "" },
	},

	// Basic peripherals
	{
		// PIC"
		{ "" },
		// DMA controller"
		{ "" },
		// Timer"
		{ "" },
		// RTC controller"
		{ "" },
		// PCI Hot-Plug controller"
		{ "" },
		// SD host controller"
		{ "" },
		// IOMMU"
		{ "" },
		// Unknown peripheral"
		{ "" },
	},

	// Input controller
	{
		// Keyboard controller
		{ "" },
		// Digitizer pen
		{ "" },
		// Mouse controller
		{ "" },
		// Scanner controller
		{ "" },
		// Gameport controller
		{ "" },
		// Unknown input controller
		{ "" },
	},

	// Docking station
	{
		// Generic docking station
		{ "" },
		// Unknown docking statio
		{ "" },
	},

	// Processor
	{
		// 386 processor
		{ "" },
		// 486 processor
		{ "" },
		// Pentium processor
		{ "" },
		// Pentium Pro processor
		{ "" },
		// Alpha processor
		{ "" },
		// PowerPC processor
		{ "" },
		// MIPS processor
		{ "" },
		// Co-Processor processor
		{ "" },
		// Unknown processor
		{ "" },
	},

	// Serial bus controller
	{
		// FireWire (IEEE 1394) controller
		{ "" },
		// ACCESS Bus controller
		{ "" },
		// SSA
		{ "" },
		// USB controller
		{ "" },
		// Fibre channel
		{ "" },
		// SMBus controller
		{ "" },
		// Infiniband controller
		{ "" },
		// IPMI interface
		{ "" },
		// SERCOS interface
		{ "" },
		// CANbus controller
		{ "" },
	},
};

const char* pci_classes[] = {
	"Unclassified",
	"Mass storage",
	"Network",
	"Video",
	"Multimedia",
	"Memory",
	"Bridge",
	"Communications",
	"Basic peripherals",
	"Input controller",
	"Docking station",
	"Processor",
	"Serial bus controller",
	"Wireless controller",
	"Intelligent controller",
	"Satellite communication controller",
	"Encyption controller",
	"Signal processing controller",
	"Processing accelerator",
	"Non-Essential instrumentation"
};

const char* pci_find_vendor(uint16_t id) {
	for (uint16_t i = 0; i < sizeof(pci_vendors); i++)
		if (pci_vendors[i].id == id)
			return pci_vendors[i].name;
	
	printk("\nError: Unknown manufacturer: %04x\n", id);
	return NULL;
}

const char* pci_find_product(uint16_t id) {
	for (uint16_t i = 0; i < sizeof(pci_products); i++)
		if (pci_products[i].id == id)
			return pci_products[i].name;
			
	printk("\nError: Unknown product: %04x\n", id);
	return NULL;
}

const char* pci_find_class(uint8_t class) {
	if (!pci_classes[class])
		printk("Error: class %d not implemented!\n", class);
	return pci_classes[class];
}

const char* pci_find_subclass(uint8_t class, uint8_t subclass) {
	if (!pci_classes[class])
		printk("Error: subclass %d in class %d not implemented!\n", subclass, class);
	return pci_subclasses[class][subclass];
}
