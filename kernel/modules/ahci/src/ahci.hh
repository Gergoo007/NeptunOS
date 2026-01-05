#pragma once

#include <types.hh>
#include <devmgr/devmgr.hh>

pstruct hba_regs {
	pstruct {
		u32 num_ports : 5;
		u32 ext_sata : 1;
		u32 enclosure_mgmt : 1;
		u32 cccs : 1;
		u32 num_cmd_slots : 5;
		u32 psc : 1;
		u32 ssc : 1;
		u32 pmd : 1;
		u32 fbss : 1;
		u32 port_multiplier : 1;
		u32 ahci_only : 1;
		u32 : 1;
		u32 max_speed : 4;
		u32 cmd_list_override : 1;
		u32 led : 1;
		u32 salp : 1;
		u32 sss : 1;
		u32 smps : 1;
		u32 ssntf : 1;
		u32 sncq : 1;
		u32 s64bit : 1;
	} caps;
	pstruct {
		u32 reset : 1;
		u32 intr : 1;
		u32 mrsm : 1;
		u32 : 28;
		u32 ahci_enable : 1;
	} ghc; // global host control
	u32 int_sts;
	u32 port_impl;
	u32 version;
	u32 ccc_ctl; // Command Completion Coalescing Control
	u32 ccc_ports; // Command Completion Coalescing Ports
	u32 em_loc;
	u32 em_ctl;
	u32 cap2;
	u32 bohc; // BIOS/OS Handoff Control and Status

	u8 pad[212];

	pstruct hba_ports {
		u64 cmd_list_base;
		u64 fis_list_base;
		u32 intr_sts;
		u32 intr_enable;
		u32 cmd_sts;
		u32 : 32;
		u32 taskfile;
		u32 sign;
		u32 sata_sts;
		u32 sata_ctl;
		u32 sata_err;
		u32 sata_active;
		u32 cmd_issue;
		u32 sata_notif;
		u32 fis_based_switch_ctl;
		u32 dev_sleep;

		u8 pad[56];
	} ports[];
};
