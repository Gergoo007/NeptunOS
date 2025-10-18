#include <arch/aarch64/aarch64.hh>

static inline u32 mmio_read(u32 o) {
	return mr32(GPIO_BASE + o);
}

static inline void mmio_write(u32 o, u32 v) {
	mw32(GPIO_BASE + o, v);
}

namespace arch {
	void init() {
		
	}

	void late_init() {
		
	}

	void halt() {
		asm volatile ("wfi");
	}

	void cli() {
		asm volatile ("spcid if");
	}

	void sti() {
		asm volatile ("cpsie if");
	}
}

void sputc(const char c) {
	mmio_write(1, 1);
}
