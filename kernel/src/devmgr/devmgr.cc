#include <devmgr/devmgr.hh>
#include <devmgr/module.hh>

vector<device_t> devices;

void devmgr_add_device(device_t&& d) {
	devices.emplace(move<device_t>(d));

	// Van modul erre az eszközre?
	for (const auto& m : modules) {
		const module_metadata_t& md = *m.metadata;
		switch (md.triggertype) {
			case ModuleTriggerTypes::ANY: break;
			case ModuleTriggerTypes::PCI_CLASS_SUBCLASS_PROGIF: {
				// Ha egyeznek, akkor mehet a fall through, tehát megnézi hogy a másik kettő stimmel-e
				if (md.trigger.PCI_CLASS_SUBCLASS_PROGIF.progif != d.props.PCI.progif)
					break;
			}
			case ModuleTriggerTypes::PCI_CLASS_SUBCLASS: {
				if (md.trigger.PCI_CLASS_SUBCLASS.class_ == d.props.PCI.class_ &&
				md.trigger.PCI_CLASS_SUBCLASS.subclass == d.props.PCI.subclass)
					modules_load(m);
				break;
			}
			case ModuleTriggerTypes::PCI_VENDOR_PRODUCT: {
				if (md.trigger.PCI_VENDOR_PRODUCT.vendor == d.props.PCI.vendor &&
				md.trigger.PCI_VENDOR_PRODUCT.product == d.props.PCI.product)
					modules_load(m);
				break;
			}
			default: {
				fatal("Invalid module trigger type: 0x%08x\n", md.triggertype);
				break;
			}
		}
	}
}
