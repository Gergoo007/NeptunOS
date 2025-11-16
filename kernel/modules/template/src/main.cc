#include <gfx/console.hh>
#include <devmgr/module.hh>

__attribute__((section(".modinfo"), used))
volatile constexpr module_metadata_t _modinfo {
	.name = "template",
	.triggertype = ModuleTriggerTypes::ANY,
	.trigger = {  }
};

void mod_main() {
	warn("template module turi ip");
}
