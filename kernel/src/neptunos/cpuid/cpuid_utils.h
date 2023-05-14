#pragma once

#include <neptunos/libk/stdall.h>

typedef struct {
	union {
		struct {
			struct {
				uint8_t cpuid_f_sse3		: 1;
				uint8_t cpuid_f_pclmul		: 1;
				uint8_t cpuid_f_dtes64		: 1;
				uint8_t cpuid_f_monitor		: 1;
				uint8_t cpuid_f_ds_cpl		: 1;
				uint8_t cpuid_f_vmx			: 1;
				uint8_t cpuid_f_smx			: 1;
				uint8_t cpuid_f_est			: 1;
				uint8_t cpuid_f_tm2			: 1;
				uint8_t cpuid_f_ssse3		: 1;
				uint8_t cpuid_f_cid			: 1;
				uint8_t cpuid_f_sdbg		: 1;
				uint8_t cpuid_f_fma			: 1;
				uint8_t cpuid_f_cx16		: 1;
				uint8_t cpuid_f_xtpr		: 1;
				uint8_t cpuid_f_pdcm		: 1;
				uint8_t cpuid_f_pcid		: 1;
				uint8_t cpuid_f_dca			: 1;
				uint8_t cpuid_f_sse4_1		: 1;
				uint8_t cpuid_f_sse4_2		: 1;
				uint8_t cpuid_f_x2apic		: 1;
				uint8_t cpuid_f_movbe		: 1;
				uint8_t cpuid_f_popcnt		: 1;
				uint8_t cpuid_f_tsc			: 1;
				uint8_t cpuid_f_aes			: 1;
				uint8_t cpuid_f_xsave		: 1;
				uint8_t cpuid_f_osxsave		: 1;
				uint8_t cpuid_f_avx			: 1;
				uint8_t cpuid_f_f16c		: 1;
				uint8_t cpuid_f_rdrand		: 1;
				uint8_t cpuid_f_hypervisor	: 1;
			} ecx;
			struct {
				uint8_t cpuid_f_fpu			: 1;
				uint8_t cpuid_f_vme			: 1;
				uint8_t cpuid_f_de			: 1;
				uint8_t cpuid_f_pse			: 1;
				uint8_t cpuid_f_tsc_edx		: 1;
				uint8_t cpuid_f_msr			: 1;
				uint8_t cpuid_f_pae			: 1;
				uint8_t cpuid_f_mce			: 1;
				uint8_t cpuid_f_cx8			: 1;
				uint8_t cpuid_f_apic		: 1;
				uint8_t cpuid_f_sep			: 1;
				uint8_t cpuid_f_mtrr		: 1;
				uint8_t cpuid_f_pge			: 1;
				uint8_t cpuid_f_mca			: 1;
				uint8_t cpuid_f_cmov		: 1;
				uint8_t cpuid_f_pat			: 1;
				uint8_t cpuid_f_pse36		: 1;
				uint8_t cpuid_f_psn			: 1;
				uint8_t cpuid_f_clflush		: 1;
				uint8_t cpuid_f_ds			: 1;
				uint8_t cpuid_f_acpi		: 1;
				uint8_t cpuid_f_mmx			: 1;
				uint8_t cpuid_f_fxsr		: 1;
				uint8_t cpuid_f_sse			: 1;
				uint8_t cpuid_f_sse2		: 1;
				uint8_t cpuid_f_ss			: 1;
				uint8_t cpuid_f_htt			: 1;
				uint8_t cpuid_f_tm			: 1;
				uint8_t cpuid_f_ia64		: 1;
				uint8_t cpuid_f_pbe			: 1;
			} edx;
		};

		struct {
			uint32_t _ecx;
			uint32_t _edx;
		} _attr_packed;
	};
} _attr_packed cpuid_features_t;

typedef struct {
	union {
		struct {
			uint8_t stepping_id 	: 4;
			uint8_t model_id 		: 4;
			uint8_t family_id 		: 4;
			uint8_t cpu_type 		: 2;
			uint8_t : 2;
			uint8_t model_id_ext 	: 4;
			uint16_t family_id_ext 	: 8;
			uint8_t : 4;
		} _attr_packed;

		struct {
			uint32_t _eax;
		} _attr_packed;
	};
} _attr_packed cpuid_model_t;

void cpuid_get_vendor_id(char* result);
void cpuid_get_features(cpuid_features_t* features);
uint8_t cpuid_get_model(void);
uint8_t cpuid_get_family(void);
uint8_t cpuid_get_current_core(void);
