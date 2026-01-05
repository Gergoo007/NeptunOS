#include <types.hh>
#include <devmgr/module.hh>
#include <devmgr/devmgr.hh>
#include <pci/pci.hh>

__attribute__((section(".modinfo"), used))
volatile constexpr module_metadata_t _modinfo {
	.name = "ahci",
	.triggertype = ModuleTriggerTypes::PCI_CLASS_SUBCLASS,
	.trigger = { .PCI_CLASS_SUBCLASS { 0x01, 0x06 } }
};

extern "C" void mod_main(device_t& _dev) {
	auto& dev = _dev.PCI;
	auto bar5 = pci_prepare_bar(_dev, 5);
	assert(!bar5.io);
	u64 mmio = bar5.addr;

	
}
