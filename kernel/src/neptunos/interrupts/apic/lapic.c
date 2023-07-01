#include "apic.h"

void lapic_init(cpu_core_t* cpu) {
	map_page(cpu->lapic_base, cpu->lapic_base, MAP_FLAGS_IO_DEFAULTS);

	volatile lapic_regs_t* lapic = (volatile lapic_regs_t*) cpu->lapic_base;
	printk("Lapic version: %08x\n", lapic->lapic_ver.max_lvt_entry);

	u32 reg = lapic->spurious_int_vector;
	reg = 0xff;
	reg |= 0x100;
	lapic->spurious_int_vector = reg;

	lapic->tpr = 0;

	lapic->timer_divide_config = 0b0001;
	
	lvt_entry_t entry = lapic->lvt_timer;
	entry.active_low = 0;
	entry.mask = 1;
	entry.vector_num = 0x51;
	entry.lvl_triggered = 1;
	entry.timer_mode = 0b01; // periodic
	lapic->lvt_timer = entry;

	lapic->timer_initial_count = 0x6000000;

	entry = lapic->lvt_lint0;
	entry.mask = 1;
	entry.nmi = cpu->nmi_pin == 0 ? 0b100 : 0;
	lapic->lvt_lint0 = entry;

	entry = lapic->lvt_lint1;
	entry.mask = 1;
	entry.nmi = cpu->nmi_pin ? 0b100 : 0;
	lapic->lvt_lint1 = entry;
}
