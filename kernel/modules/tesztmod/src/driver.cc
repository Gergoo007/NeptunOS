#include <gfx/console.hh>
#include <devmgr/module.hh>

__attribute__((section(".modinfo"), used))
volatile constexpr module_metadata_t _modinfo {
	.name = "tesztmod",
	.triggertype = ModuleTriggerTypes::ANY,
	.trigger = {  }
};

extern "C" void mod_main() {
	warn("tesztmod loaded successfully!");
}
