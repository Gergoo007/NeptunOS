#include <pci/pci.hh>
#include <arch/amd64/io.hh>
#include <arch/amd64/paging.hh>
#include <devmgr/devmgr.hh>

#define numbits(x) ((1ULL << (x)) - 1)

static constexpr u16 CFG_ADDR = 0xcf8;
static constexpr u16 CFG_DATA = 0xcfc;
mcfg_t* mcfg;

u32 pci_read32(u8 bus, u8 slot, u8 func, u8 offset) {
	if (offset & 3)
		error("unaligned read!");

	if (mcfg) {
		u64 mmio = VIRTUAL((u64)mcfg->cfg_spaces[0].base);
		mmio += (bus << 20) + (slot << 15) + (func << 12) + offset;
		check_page(mmio, UC);
		return *(volatile u32*)mmio;
	} else {
		u32 address;

		address = (u32)((((u32)(bus)) << 16) | (((u32)(slot)) << 11) |
				(((u32)(func)) << 8) | offset | 0x80000000UL);

		outl(CFG_ADDR, address);
		return inl(CFG_DATA);
	}
}

void pci_write32(u8 bus, u8 slot, u8 func, u8 offset, u32 data) {
	if (offset & 3)
		error("unaligned read!");

	if (mcfg) {
		u64 mmio = VIRTUAL((u64)mcfg->cfg_spaces[0].base);
		mmio += (bus << 20) + (slot << 15) + (func << 12) + offset;
		check_page(mmio, UC);
		*(volatile u32*)mmio = data;
	} else {
		u32 address;

		address = (u32)((((u32)(bus)) << 16) | (((u32)(slot)) << 11) |
				(((u32)(func)) << 8) | offset | 0x80000000UL);

		outl(CFG_ADDR, address);
		outl(CFG_DATA, data);
	}
}

u32 pci_read(u8 bus, u8 slot, u8 func, u8 offset, u8 bits = 32) {
	u32 realoffset = align_down(offset, 4);
	if (align_down(offset, 4) != align_down(offset + (bits/8) - 1, 4))
		error("overlapping reads not supported: %d bits @ %08x", bits, offset);
	u32 data = pci_read32(bus, slot, func, realoffset);
	data >>= (offset - realoffset) * 8;
	data &= numbits(bits);
	return data;
}

void pci_write(u8 bus, u8 slot, u8 func, u8 offset, u32 data, u8 bits = 32) {
	u32 realoffset = align_down(offset, 4);
	if (align_down(offset, 4) != align_down(offset + (bits/8) - 1, 4))
		error("overlapping writes not supported: %d bits @ %08x", bits, offset);
	u32 value = pci_read32(bus, slot, func, realoffset);
	u32 new_value = value & ~numbits(bits);
	new_value |= data << ((offset & 3) * 8);
	pci_write32(bus, slot, func, realoffset, new_value);
}

u32 pci_read(u8 bus, u8 slot, u8 func, pci_register_t reg) {
	u32 realoffset = align_down(reg.offset, 4);
	if (align_down(reg.offset, 4) != align_down(reg.offset + (reg.bits/8) - 1, 4))
		error("overlapping reads not supported: %d bits @ %08x", reg.bits, reg.offset);
	u32 data = pci_read32(bus, slot, func, realoffset);
	data >>= (reg.offset - realoffset) * 8;
	data &= numbits(reg.bits);
	return data;
}

void pci_write(u8 bus, u8 slot, u8 func, pci_register_t reg, u32 data) {
	u32 realoffset = align_down(reg.offset, 4);
	if (align_down(reg.offset, 4) != align_down(reg.offset + (reg.bits/8) - 1, 4))
		error("overlapping writes not supported: %d bits @ %08x", reg.bits, reg.offset);
	u32 value = pci_read32(bus, slot, func, realoffset);
	u32 new_value = value & ~numbits(reg.bits);
	new_value |= data << ((reg.offset & 3) * 8);
	pci_write32(bus, slot, func, realoffset, new_value);
}

u32 pci_read(device_t& dev, pci_register_t reg) { return pci_read(dev.kinds.get<device_t_PCI>().bus, dev.kinds.get<device_t_PCI>().dev, dev.kinds.get<device_t_PCI>().fun, reg); }
void pci_write(device_t& dev, pci_register_t reg, u32 data) { pci_write(dev.kinds.get<device_t_PCI>().bus, dev.kinds.get<device_t_PCI>().dev, dev.kinds.get<device_t_PCI>().fun, reg, data); }

void check_bus(u8 bus) {
	for (u32 j = 0; j < 32; j++) {
		for (u8 i = 0; i < 8; i++) {
			if (pci_read(bus, j, i, PciRegs::VENDOR) == 0xffff)
				continue;

			u32 hdrt = pci_read(bus, j, i, PciRegs::HDRTYPE);
		
			devmgr_add_device(device_t {
				.extra = nullptr,
				.subsys = DevmgrSubsys::PCI,
				.kinds = devunion_t(true, device_t_PCI {
					.vendor = (u16)pci_read(bus, j, i, PciRegs::VENDOR),
					.product = (u16)pci_read(bus, j, i, PciRegs::PRODUCT),
					.class_ = (u8)pci_read(bus, j, i, PciRegs::CLASS),
					.subclass = (u8)pci_read(bus, j, i, PciRegs::SUBCLASS),
					.progif = (u8)pci_read(bus, j, i, PciRegs::PROGIF),
					.bus = bus,
					.dev = (u8)j,
					.fun = i,
				})
			});

			if ((hdrt & 3) == 1) {
				u8 secondary = pci_read(bus, j, i, PciRegs::SECONDARYBUS);
				check_bus(secondary);
			}
		}
	}
}

void pci_init() {
	if (mcfg) {
		u32 num_cfg_spaces = (mcfg->length - 36) / sizeof(mcfg->cfg_spaces[0]);
		if (num_cfg_spaces > 1)
			fatal("only 1 segment group supproted");
	}

	for (u32 fun = 0; fun < 8; fun++) {
		if (pci_read(0, 0, fun, PciRegs::VENDOR) == 0xffff)
			continue;

		u8 cl = pci_read(0, 0, fun, PciRegs::CLASS);
		u8 scl = pci_read(0, 0, fun, PciRegs::SUBCLASS);
		debug("root found %d %d", cl, scl);
		if (cl == 6 && scl == 4) {
			debug("pci2pci @ %d & %d", pci_read(0, 0, fun, PciRegs::PRIMARYBUS), pci_read(0, 0, fun, PciRegs::SECONDARYBUS));
			check_bus(pci_read(0, 0, fun, PciRegs::PRIMARYBUS));
			check_bus(pci_read(0, 0, fun, PciRegs::SECONDARYBUS));
		} else if (cl == 6 && scl == 0) {
			debug("host bridge @ %d", fun);
			check_bus(fun);
		} else {
			warn("pci root func unknown %02x:%02x.%01x (class 0x%x 0x%x)", 0, 0, fun, cl, scl);
		}
	}
}

void pci_enable_bus_mastering(device_t& dev) {
	u32 pcicmd = pci_read(dev, PciRegs::CMD);
	pcicmd |= 0b100; // bus master
	pcicmd |= 0b010; // mem access
	pcicmd |= 0b001; // io access
	pci_write(dev, PciRegs::CMD, pcicmd);
}

pci_bar pci_prepare_bar(device_t& dev, u8 barnum) {
	auto barreg = pci_register_t(barnum);
	u64 addr = pci_read(dev, barreg), orig = addr;
	pci_bar ret;
	ret.io = addr & 1;
	u8 type = (addr >> 1) & 0b11;
	bool prefetchable = addr & (1 << 3);
	addr &= ~0b1111;
	if (ret.io) {
		ret.addr = addr;
	} else {
		pci_write(dev, barreg, 0xffffffff);
		u64 size = (~(pci_read(dev, barreg) & ~0xf)) + 1;
		pci_write(dev, barreg, orig);

		if (type == 2) {
			if (barnum == 5) fatal("BAR5 is 64 bit? Device %04x:%04x", dev.kinds.get<device_t_PCI>().vendor, dev.kinds.get<device_t_PCI>().product);
			u64 addr2 = pci_read(dev, pci_register_t(barnum + 1));
			addr2 &= ~0b1111ULL;
			addr |= (addr2 << 32);
		}

		for (u32 i = 0; i < align(size, 0x1000); i += 0x1000)
			map_page(VIRTUAL(addr) + i, addr + i, 0b11, prefetchable ? MCACHE::WT : MCACHE::UC);

		ret.addr = VIRTUAL(addr);
	}

	return ret;
}
