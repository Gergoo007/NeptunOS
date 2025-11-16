#include <gfx/console.hh>
#include <devmgr/module.hh>

__attribute__((section(".modinfo"), used))
volatile constexpr module_metadata_t _modinfo {
	.name = "uhci",
	.triggertype = ModuleTriggerTypes::PCI_CLASS_SUBCLASS_PROGIF,
	.trigger = { .PCI_CLASS_SUBCLASS_PROGIF { 0xc, 0x3, 0x0 } }
};

void mod_main() {
	warn("UHCI loaded successfully!");
}
