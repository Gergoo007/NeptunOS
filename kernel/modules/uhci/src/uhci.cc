#include <gfx/console.hh>
#include <devmgr/module.hh>
#include <devmgr/devmgr.hh>
#include <pci/pci.hh>
#include <arch/amd64/amd64.hh>

#include "uhci.hh"

__attribute__((section(".modinfo"), used))
volatile constexpr module_metadata_t _modinfo {
	.name = "uhci",
	.triggertype = ModuleTriggerTypes::PCI_CLASS_SUBCLASS_PROGIF,
	.trigger = { .PCI_CLASS_SUBCLASS_PROGIF { 0xc, 0x3, 0x0 } }
};

extern "C" void mod_main(device_t* dev) {
	u16 io = pci_read(*dev, PciRegs::BAR4);

	assert(io & 1);

	// Az alsó 2 bit metadata, le kell azokat szedni
	io &= ~0b11u;

	// TODO: legacy support kikapcs?

	// Bus mastering bekapcs
	u32 pcicmd = pci_read(*dev, PciRegs::CMD);
	pcicmd |= 1 << 2;
	pci_write(*dev, PciRegs::CMD, pcicmd);

	

	report("uhci port: %04x", io);

	// pause();
}
