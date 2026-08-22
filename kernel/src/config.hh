#pragma once

#define SERIALPRINTK

#define VECTOR_NULLBYDEFAULT

constexpr bool DBG = true;

constexpr unsigned long VMM_REDZONE_SIZE = 64;
constexpr unsigned char VMM_REDZONE_MAGIC = 0xbc;
constexpr unsigned long long VMM_REDZONE_CHECK_PERIOD = 30; // ms

constexpr bool STACKTRACE_ON_FATAL = true;
constexpr bool IOAPIC_FIX = true;
constexpr unsigned long long VEC_DEFAULT_SIZE = 0;
constexpr unsigned long long HASHMAP_DEFAULT_SIZE = 64;
static_assert(HASHMAP_DEFAULT_SIZE);

constexpr unsigned int SCHED_MAX_PROCESSES = 1024;
constexpr unsigned int SCHED_QUANTUM = 100;

constexpr unsigned int INTR_STACK_SIZE = 0x200000;
constexpr unsigned int SYSCALL_STACK_SIZE = 0x200000;
constexpr unsigned int USER_STACK_SIZE = 0x200000;

static constexpr const char* debug_files[] = {
	"ehci.cc",
	"usb.cc",
	"hub.cc",

	"fat.cc",
	
	"acpi.cc",
	"aml.cc"
};

// Lowest log level that will get printed; 0 is debug, 5 is fatal
static constexpr unsigned int loglevel = 0;

static constexpr unsigned char cfg_strcmp(const char* s1, const char* s2) {
	while (*s1 && *s2) {
		if (*s1 != *s2)
			return 1;
		s1++;
		s2++;
	}
	if (*s1 != *s2) return 1;
	return 0;
}

static constexpr bool cfg_willitprint(unsigned int lvl, const char* file) {
	if (lvl < loglevel) return false;
	// The List only applies to debug prints
	if (lvl > 0) return true;
	for (unsigned long i = 0; i < sizeof(debug_files) / 8; i++) {
		if (!cfg_strcmp(debug_files[i], file))
			return true;
	}
	return false;
}
