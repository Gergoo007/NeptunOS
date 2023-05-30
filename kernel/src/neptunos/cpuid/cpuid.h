#pragma once

#include <neptunos/libk/stdall.h>

typedef struct {
	struct {
		u8 cpuid_f_sse3		: 1;
		u8 cpuid_f_pclmul	: 1;
		u8 cpuid_f_dtes64	: 1;
		u8 cpuid_f_monitor	: 1;
		u8 cpuid_f_ds_cpl	: 1;
		u8 cpuid_f_vmx		: 1;
		u8 cpuid_f_smx		: 1;
		u8 cpuid_f_est		: 1;
		u8 cpuid_fm2		: 1;
		u8 cpuid_f_ssse3	: 1;
		u8 cpuid_f_cid		: 1;
		u8 cpuid_f_sdbg		: 1;
		u8 cpuid_f_fma		: 1;
		u8 cpuid_f_cx16		: 1;
		u8 cpuid_f_xtpr		: 1;
		u8 cpuid_f_pdcm		: 1;
		u8 cpuid_f_pcid		: 1;
		u8 cpuid_f_dca		: 1;
		u8 cpuid_f_sse4_1	: 1;
		u8 cpuid_f_sse4_2	: 1;
		u8 cpuid_f_x2apic	: 1;
		u8 cpuid_f_movbe	: 1;
		u8 cpuid_f_popcnt	: 1;
		u8 cpuid_fsc		: 1;
		u8 cpuid_f_aes		: 1;
		u8 cpuid_f_xsave	: 1;
		u8 cpuid_f_osxsave	: 1;
		u8 cpuid_f_avx		: 1;
		u8 cpuid_f_f16c		: 1;
		u8 cpuid_f_rdrand	: 1;
		u8 cpuid_f_hypervisor	: 1;
	} ecx;
	struct {
		u8 cpuid_f_fpu		: 1;
		u8 cpuid_f_vme		: 1;
		u8 cpuid_f_de		: 1;
		u8 cpuid_f_pse		: 1;
		u8 cpuid_fsc_edx	: 1;
		u8 cpuid_f_msr		: 1;
		u8 cpuid_f_pae		: 1;
		u8 cpuid_f_mce		: 1;
		u8 cpuid_f_cx8		: 1;
		u8 cpuid_f_apic		: 1;
		u8 cpuid_f_sep		: 1;
		u8 cpuid_f_mtrr		: 1;
		u8 cpuid_f_pge		: 1;
		u8 cpuid_f_mca		: 1;
		u8 cpuid_f_cmov		: 1;
		u8 cpuid_f_pat		: 1;
		u8 cpuid_f_pse36	: 1;
		u8 cpuid_f_psn		: 1;
		u8 cpuid_f_clflush	: 1;
		u8 cpuid_f_ds		: 1;
		u8 cpuid_f_acpi		: 1;
		u8 cpuid_f_mmx		: 1;
		u8 cpuid_f_fxsr		: 1;
		u8 cpuid_f_sse		: 1;
		u8 cpuid_f_sse2		: 1;
		u8 cpuid_f_ss		: 1;
		u8 cpuid_f_htt		: 1;
		u8 cpuid_fm			: 1;
		u8 cpuid_f_ia64		: 1;
		u8 cpuid_f_pbe		: 1;
	} edx;
} _attr_packed cpuid_eax_1;

typedef struct {
	u8 stepping_id	: 4;
	u8 model_id		: 4;
	u8 family_id	: 4;
	u8 cpuype		: 2;
	u8 : 2;
	u8 model_id_ext : 4;
	u16 family_id_ext	: 8;
	u8 : 4;
} _attr_packed cpuid_version_t;

u32 cpuid_get_vendor(char* vend);
cpuid_version_t cpuid_get_version(void);
u8 cpuid_sse(void);
u8 cpuid_sse2(void);
u8 cpuid_sse3(void);
u8 cpuid_sse41(void);
u8 cpuid_sse42(void);
u8 cpuid_avx(void);
u8 cpuid_avx2(void);
u8 cpuid_avx512f(void);
