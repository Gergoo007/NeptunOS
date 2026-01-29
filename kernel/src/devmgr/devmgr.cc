#include <devmgr/devmgr.hh>
#include <devmgr/module.hh>

vector<unique_ptr<device_t>> devices;

device_t& devmgr_add_device(device_t&& _d) {
	device_t& d = *devices.emplace_back(move<device_t>(_d));

	// Van modul erre az eszközre?
	for (auto& m : modules) {
		const module_metadata_t& md = *m.metadata;
		switch (d.subsys) {
			case DevmgrSubsys::PCI: {
				if (((u32)m.metadata->triggertype & (u32)ModuleTriggerTypes::PCIMASK) == 0) continue;
				auto& pcidev = d.kinds.get<device_t_PCI>();

				switch (md.triggertype) {
					case ModuleTriggerTypes::PCI_CLASS_SUBCLASS_PROGIF: {
						// Ha egyeznek, akkor mehet a fall through, tehát megnézi hogy a másik kettő stimmel-e
						if (md.trigger.PCI_CLASS_SUBCLASS_PROGIF.progif != pcidev.progif)
							break;
					}
					// fall through
					case ModuleTriggerTypes::PCI_CLASS_SUBCLASS: {
						if (md.trigger.PCI_CLASS_SUBCLASS.class_ == pcidev.class_ &&
						md.trigger.PCI_CLASS_SUBCLASS.subclass == pcidev.subclass)
							modules_launch(m, d);
						break;
					}
					case ModuleTriggerTypes::PCI_VENDOR_PRODUCT: {
						if (md.trigger.PCI_VENDOR_PRODUCT.vendor == pcidev.vendor &&
						md.trigger.PCI_VENDOR_PRODUCT.product == pcidev.product)
							modules_launch(m, d);
						break;
					}

					default: break;
				}
			}

			case DevmgrSubsys::USB: break;
			case DevmgrSubsys::MSD: break;
			case DevmgrSubsys::NONE: break;
		}
	}

	switch (d.subsys) {
		case DevmgrSubsys::MSD:
			msd_scout(d);
			break;
		default: break;
	}

	return d;
}

void devmgr_add_fs(filesystem& f) {
	for (auto& m : modules) {
		if (m.metadata->triggertype == ModuleTriggerTypes::FILESYSTEM)
			modules_launch_fs(m, f);
	}
}
