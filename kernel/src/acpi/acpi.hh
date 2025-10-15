#pragma once

#include <types.hh>
#include <pci/pci.hh>

namespace acpi {
	pstruct gas {
		u8 addr_space; // 0 - sys mem, 1 - io mem
		u8 reg_bit_width;
		u8 reg_bit_offset;
		u8 access_size;
		u64 address;
	};

	pstruct SDT {
		// char sign[4];
		u32 sign;
		u32 length;
		u8 rev;
		u8 checksum;
		char oemid[6];
		char oemtableid[8];
		u32 oem_rev;
		u32 creatorid;
		u32 creatorrev;
	};

	pstruct RootPointer {
		char sign[8];
		u8 checksum;
		char oemid[6];
		u8 rev;
		u32 rsdt;      // deprecated since version 2.0

		u32 length;
		u64 xsdt;
		u8 extchecksum;
		u8 reserved[3];
	};

	pstruct RootTable {
		SDT hdr;
		punion {
			u32 rsdtptrs[];
			u64 xsdtptrs[];
		};
	};

	pstruct FADT {
		SDT hdr;
		u32 fw_ctl;
		u32 dsdt;
		u8 : 8;
		u8 preferred_pm_prof;
		u16 sci_int;
		u32 smi_cmd_port;
		u8 acpi_enable;
		u8 acpi_disable;
		u8 s4bios_req;
		u8 pstate_cnt;
		u32 pm1a_evt_blk;
		u32 pm1b_evt_blk;
		u32 pm1a_ctl_blk;
		u32 pm1b_ctl_blk;
		u32 pm2_ctl_blk;
		u32 pm_timer;
		u32 gpe0;
		u32 gpe1;
		u32 : 24;
		u8 pm_timer_bytes;
		u8 r0[20];
		punion {
			pstruct {
				u8 wbinvd : 1;
				u8 wbinvd_flush : 1;
				u8 proc_c1 : 1;
				u8 p_lvl2_up : 1;
				u8 power_btn : 1;
				u8 sleep_btn : 1;
				u8 fix_rtc : 1;
				u8 rtc_s4 : 1;
				u8 tmr_val_ext : 1;
				u8 dck_cap : 1;
				u8 reset_reg_sup : 1;
				u8 sealed_case : 1;
				u8 headless : 1;
				u8 cpu_sw_slp : 1;
				u8 pci_exp_wak : 1;
			};
			u32 raw;
		} flags;
		u8 r1[15];
		u8 fadt_minor;
		u64 x_fw_ctl;
		u64 x_dsdt;
		gas x_pm1a_evt_blk;
		gas x_pm1b_evt_blk;
		gas x_pm1a_ctl_blk;
		gas x_pm1b_ctl_blk;
		gas x_pm2_ctl_blk;
		gas x_pm_timer;
		gas x_gpe0;
		gas x_gpe1;
		gas sleep_ctl;
		gas sleep_sts;
		char hypervisor_id[8];
	};

	// Ezek az okostojások nem engedik az implicit cast-et
	// struct enum-oknál
	struct SIGNS {
		static constexpr u32 FADT = 'PCAF';
		static constexpr u32 MCFG = 'GFCM';
	};

	bool validate(SDT* table);
	u8 validateRsdp(RootPointer* table);
	void init();
}
