#include <arch/amd64/amd64.hh>
#include <arch/amd64/idt.hh>
#include <arch/amd64/gdt.hh>
#include <arch/amd64/tss.hh>
#include <arch/amd64/io.hh>
#include <arch/amd64/pit.hh>
#include <arch/amd64/cpuid.hh>
#include <arch/amd64/apic.hh>
#include <mm/vmm.hh>

#include <arch/arch.hh>

#define PORT 0x3f8

void arch_init(bool bsp) {
	sse_init();
	if (bsp)
		sinit();
}

void arch_late_init(bool bsp) {
	wm_cursor = (u64)pmm_alloc();
	wm_free = pmm_pagesize;

	auto* gdt = arch_gdt_init();
	arch_tss_init(gdt);
	arch_idt_init();

	if (bsp)
		arch_pit_init();

	thread_info* thr = (thread_info*)wm_alloc(sizeof(thread_info));
	amd64_set_msr(Amd64Msrs::FSBase, (u64)thr);
}

void arch_halt() { asm volatile ("hlt"); }
void arch_cli() { asm volatile ("cli"); }
void arch_sti() { asm volatile ("sti"); }

void sinit() {
	outb(PORT + 1, 0x00);
	outb(PORT + 3, 0x80);
	outb(PORT + 0, 0x03);
	outb(PORT + 1, 0x00);
	outb(PORT + 3, 0x03);
	outb(PORT + 2, 0xC7);
	// outb(PORT + 4, 0x0B);
	outb(PORT + 4, 0x1E);
	outb(PORT + 0, 0xAE);

	outb(PORT + 4, 0x0F);
}

void sputc(const char c) {
	while ((inb(PORT + 5) & 0x20) == 0);
	outb(PORT, c);
	if (c == '\n')
		sputc('\r');
}

char sgetc() {
	while ((inb(PORT + 5) & 1) == 0);
	return inb(PORT);
}

void arch_sleep(u64 ms, bool skippable) {
	// Emulátoron nem kell várni a hardverre
	if (skippable && cpuid_is_emu()) return;
	u64 end = tmr_counter + ms;
	while (tmr_counter < end) arch_halt();
}

// TODO: thread safety!!
static u64 timer;
void arch_start_timer() {
	timer = tmr_counter;
}

u64 arch_ms_passed() {
	return tmr_counter - timer;
}

bool arch_elapsed(u64 ms) {
	return tmr_counter > timer + ms;
}

void arch_assign_irq(u32 irq, void (*handler)(cpu_state_t* frame)) {
	u8 vector = idt_allocate_vector(handler);
	arch_ioapic_initialize_irq(irq, vector, IoapicDelivmode::FIXED, cpuid_xapic_id());
	arch_ioapic_mask_irq(irq, 0);
}

u32 arch_alloc_isr(void (*handler)(cpu_state_t* frame)) { return idt_allocate_vector(handler); }
void arch_eoi() { arch_lapic_eoi(); }
