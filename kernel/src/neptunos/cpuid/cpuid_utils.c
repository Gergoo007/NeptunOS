#include <neptunos/libk/stdall.h>
#include <neptunos/cpuid/cpuid_utils.h>

void cpuid_get_vendor_id(char* result) {
	asm("movl $0x0, %eax");
	asm("cpuid");
	asm("movl %%ebx, %0" : "=b"(*(uint32_t*)(&result[0])));
	asm("movl %%ecx, %0" : "=c"(*(uint32_t*)(&result[8])));
	asm("movl %%edx, %0" : "=d"(*(uint32_t*)(&result[4])));
	
	result[12] = '\0';
}

void cpuid_get_features(cpuid_features_t* features) {
	asm("movl $0x1, %eax");
	asm("cpuid");
	asm("movl %%edx, %0" : "=d"(features->_edx));
	asm("movl %%ecx, %0" : "=c"(features->_ecx));
}

uint8_t cpuid_get_family(void) {
	cpuid_model_t modelinfo;
	asm("movl $0x1, %eax");
	asm("cpuid");
	asm("movl %%eax, %0" : "=a"(modelinfo));
	
	if (modelinfo.family_id == 15) {
		return modelinfo.family_id + modelinfo.family_id_ext;
	} else {
		return modelinfo.family_id;
	}
}

uint8_t cpuid_get_model(void) {
	cpuid_model_t modelinfo;
	asm("movl $0x1, %eax");
	asm("cpuid");
	asm("movl %%eax, %0" : "=a"(modelinfo));
	
	if (modelinfo.family_id == 6 || modelinfo.family_id == 15) {
		return (modelinfo.model_id_ext << 4) + modelinfo.model_id;
	} else {
		return modelinfo.model_id;
	}
}

uint8_t cpuid_get_current_core(void) {
	cpuid_features_t feats;
	asm("movl $0x1, %eax");
	asm("cpuid");
	asm("movl %%edx, %0" : "=d"(feats._edx));
	asm("movl %%ecx, %0" : "=c"(feats._ecx));
	return feats.ecx.cpuid_f_pcid;
}
