#include <arch/amd64/pit.hh>
#include <arch/arch.hh>
#include <arch/amd64/io.hh>

void arch_pit_init() {
	arch_cli();

	pit_cmd_t cmd = {
		.bcd = 0,
		.operation = PIT_RATE_GEN,
		.access = PIT_LOHIBYTE,
		.ch = 0,
	};

	outb(PIT_IO_CMD, cmd.raw);

	// Egy 'katt' 1 ms
	u16 count = 1250;

	outb(PIT_IO_CH0, count & 0x00ff);
	outb(PIT_IO_CH0, (count & 0xff00) >> 8);

	// ioapic_set_destination(ioapic_irqs[0], 0, 0);
	// ioapic_set_vector(ioapic_irqs[0], 0x41);
	// ioapic_set_mask(ioapic_irqs[0], 0);

	arch_sti();
}
