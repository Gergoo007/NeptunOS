#include "pci_ids.h"

const pci_id_pair pci_vendors[] = {
	{ 0x8086, "Intel" },
	{ 0x1002, "AMD/ATI" },
	{ 0x1b21, "ASMedia" },
	{ 0x1234, "Bochs" },
	{ 0x0951, "Kingston Technology" },
	{ 0x055d, "Samsung Electro-Mechanics Co." },
	{ 0x1d27, "ASUS" },
	{ 0x10de, "NVIDIA Corporation" },
};

const pci_id_pair pci_products[] = {
	{ 0x29c0, "82G33/G31/P35/P31 Express DRAM Controller" },
	{ 0x1111, "Virtual graphics" },
	{ 0x2918, "82801IB (ICH9) LPC Interface Controller" },
	{ 0x2922, "82801IR/IO/IH (ICH9R/DO/DH) 6 port SATA Controller [AHCI mode]" },
	{ 0x2930, "82801I (ICH9 Family) SMBus Controller" },
	{ 0x0c00, "4th Gen Core Processor DRAM Controller" },
	{ 0x0c01, "Xeon E3-1200 v3/4th Gen Core Processor PCI Express x16 Controller" },
	{ 0x8cb1, "9 Series Chipset Family USB xHCI Controller" },
	{ 0x15a1, "Ethernet Connection (2) I218-V" },
	{ 0x8cad, "9 Series Chipset Family USB EHCI Controller #2" },
	{ 0x8c90, "9 Series Chipset Family PCI Express Root Port 1" },
	{ 0x244e, "82801 PCI Bridge" },
	{ 0x8ca6, "9 Series Chipset Family USB EHCI Controller #1" },
	{ 0x8cc4, "Z97 Chipset LPC Controller" },
	{ 0x8c82, "9 Series Chipset Family SATA Controller [AHCI Mode]" },
	{ 0x8ca2, "9 Series Chipset Family SMBus Controller" },
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

static char buf[16];
const char* pci_find_vendor(uint16_t id) {
	for (uint16_t i = 0; i < sizeof(pci_vendors); i++)
		if (pci_vendors[i].id == id)
			return pci_vendors[i].name;

	sprintf(buf, "unkn_vend(%04x)", id);
	return buf;
}

const char* pci_find_product(uint16_t id) {
	for (uint16_t i = 0; i < sizeof(pci_products); i++)
		if (pci_products[i].id == id)
			return pci_products[i].name;
			
	sprintf(buf, "unkn_prod(%04x)", id);
	return buf;
}

const char* pci_find_class(uint8_t class) {
	if (pci_classes[class])
		return pci_classes[class];

	sprintf(buf, "unkn_clas(%04x)", class);
	return buf;
}

const char* pci_find_subclass(uint8_t class, uint8_t subclass) {
	if (pci_subclasses[class][subclass])
		return pci_subclasses[class][subclass];

	sprintf(buf, "unkn_subc(%04x->%04x)", class, subclass);
	return buf;
}
