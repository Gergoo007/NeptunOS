#include <pci/pci.hh>
#include <arch/amd64/io.hh>
#include <arch/amd64/paging.hh>

#define numbits(x) ((1ULL << (x)) - 1)

namespace pci {
	constexpr u16 CFG_ADDR = 0xcf8;
	constexpr u16 CFG_DATA = 0xcfc;
	MCFG* mcfg;

	u32 pci_read32(u8 bus, u8 slot, u8 func, u8 offset) {
		if (offset & 3)
			error("unaligned read!\n");

		if (mcfg) {
			u64 mmio = VIRTUAL((u64)mcfg->cfg_spaces[0].base);
			mmio += (bus << 20) + (slot << 15) + (func << 12) + offset;
			arch::check_page(mmio);
			return *(volatile u32*)mmio;
		} else {
			u32 address;

			address = (u32)((u32(bus) << 16) | (u32(slot) << 11) |
					(u32(func) << 8) | offset | 0x80000000UL);

			arch::outl(CFG_ADDR, address);
			return arch::inl(CFG_DATA);
		}
	}

	void pci_write32(u8 bus, u8 slot, u8 func, u8 offset, u32 data) {

	}

	u32 pci_read(u8 bus, u8 slot, u8 func, u8 offset, u8 bits = 32) {
		u32 realoffset = align_down(offset, 4);
		if (align_down(offset, 4) != align_down(offset + (bits/8) - 1, 4))
			error("overlapping reads not supported: %d bits @ %08x\n", bits, offset);
		u32 data = pci_read32(bus, slot, func, realoffset);
		data >>= (offset - realoffset) * 8;
		data &= numbits(bits);
		return data;
	}

	void pci_write(u8 bus, u8 slot, u8 func, u8 offset, u32 data, u8 bits = 32) {
		u32 realoffset = align_down(offset, 4);
		if (align_down(offset, 4) != align_down(offset + (bits/8) - 1, 4))
			error("overlapping writes not supported: %d bits @ %08x\n", bits, offset);
		u32 value = pci_read32(bus, slot, func, realoffset);
		u32 new_value = value & ~numbits(bits);
		new_value |= data << ((offset & 3) * 8);
		pci_write32(bus, slot, func, realoffset, new_value);
	}

	u32 pci_read(u8 bus, u8 slot, u8 func, Register reg) {
		u32 realoffset = align_down(reg.offset, 4);
		if (align_down(reg.offset, 4) != align_down(reg.offset + (reg.bits/8) - 1, 4))
			error("overlapping reads not supported: %d bits @ %08x\n", reg.bits, reg.offset);
		u32 data = pci_read32(bus, slot, func, realoffset);
		data >>= (reg.offset - realoffset) * 8;
		data &= numbits(reg.bits);
		return data;
	}

	void pci_write(u8 bus, u8 slot, u8 func, Register reg, u32 data) {
		u32 realoffset = align_down(reg.offset, 4);
		if (align_down(reg.offset, 4) != align_down(reg.offset + (reg.bits/8) - 1, 4))
			error("overlapping writes not supported: %d bits @ %08x\n", reg.bits, reg.offset);
		u32 value = pci_read32(bus, slot, func, realoffset);
		u32 new_value = value & ~numbits(reg.bits);
		new_value |= data << ((reg.offset & 3) * 8);
		pci_write32(bus, slot, func, realoffset, new_value);
	}

	void check_device(u8 bus, u8 dev) {
		for (u32 i = 0; i < 8; i++) {
			if (pci_read(bus, dev, i, Regs::VENDOR) == 0xffff)
				continue;
			report(
				"device '%02x:%02x:%01x' %04x:%04x class %x %x\n",
				bus, dev, i,
				pci_read(bus, dev, i, Regs::VENDOR),
				pci_read(bus, dev, i, Regs::PRODUCT),
				pci_read(bus, dev, i, Regs::CLASS),
				pci_read(bus, dev, i, Regs::SUBCLASS)
			);
		}
	}

	void check_bus(u8 bus) {
		for (u32 i = 0; i < 32; i++) {
			if (pci_read(bus, i, 0, Regs::VENDOR) == 0xffff) continue;
			check_device(bus, i);
		}
	}

	void init() {
		if (mcfg) {
			u32 num_cfg_spaces = (mcfg->sdt.length - sizeof(mcfg->sdt)) / sizeof(mcfg->cfg_spaces[0]);
			if (num_cfg_spaces > 1)
				fatal("only 1 segment group supproted\n");
		}

		for (u32 fun = 0; fun < 8; fun++) {
			if (pci_read(0, 0, fun, Regs::VENDOR) == 0xffff)
				continue;
			u8 cl = pci_read(0, 0, fun, Regs::CLASS);
			u8 scl = pci_read(0, 0, fun, Regs::SUBCLASS);
			if (cl == 6 && scl == 4) {
				printk("detected pci2pci bridge %d\n", fun);
				check_bus(pci_read(0, 0, fun, Regs::PRIMARYBUS));
				check_bus(pci_read(0, 0, fun, Regs::SECONDARYBUS));
			} else if (cl == 6 && scl == 0) {
				printk("detected pci host bridge %d\n", fun);
				check_bus(fun);
			} else {
				warn("pci root func unknown %02x:%02x.%01x (class 0x%x 0x%x)\n", 0, 0, fun, cl, scl);
			}
		}
	}
}
