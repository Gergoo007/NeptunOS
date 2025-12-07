#include <devmgr/devmgr.hh>
#include <devmgr/module.hh>

vector<unique_ptr<device_t>> devices;

device_t& devmgr_add_device(device_t&& _d) {
	device_t& d = *devices.emplace((move<device_t>(_d)));

	// Van modul erre az eszközre?
	for (auto& m : modules) {
		const module_metadata_t& md = *m.metadata;
		switch (md.triggertype) {
			case ModuleTriggerTypes::ANY: break;
			case ModuleTriggerTypes::PCI_CLASS_SUBCLASS_PROGIF: {
				// Ha egyeznek, akkor mehet a fall through, tehát megnézi hogy a másik kettő stimmel-e
				if (md.trigger.PCI_CLASS_SUBCLASS_PROGIF.progif != d.PCI.progif)
					break;
			}
			case ModuleTriggerTypes::PCI_CLASS_SUBCLASS: {
				if (md.trigger.PCI_CLASS_SUBCLASS.class_ == d.PCI.class_ &&
				md.trigger.PCI_CLASS_SUBCLASS.subclass == d.PCI.subclass)
					modules_launch(m, d);
				break;
			}
			case ModuleTriggerTypes::PCI_VENDOR_PRODUCT: {
				if (md.trigger.PCI_VENDOR_PRODUCT.vendor == d.PCI.vendor &&
				md.trigger.PCI_VENDOR_PRODUCT.product == d.PCI.product)
					modules_launch(m, d);
				break;
			}
			default: {
				fatal("Invalid module trigger type: 0x%08x\n", md.triggertype);
				break;
			}
		}
	}

	return d;
}
