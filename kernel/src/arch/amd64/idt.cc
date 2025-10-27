#include <arch/amd64/idt.hh>
#include <arch/amd64/paging.hh>
#include <mm/vmm.hh>

#define print_reg(st, reg, reg2) error("%s: %p  %s: %p", #reg, (void*)st->reg, #reg2, (void*)st->reg2)

extern "C" void onInterrupt(arch::idt::cpu_regs* frame) {
	if (frame->exc == EXC::PF) {
		if ((frame->cr2 & 0xffff900000000000) == 0xffff900000000000) {
			// this s2M flag cost me a piece of my soul
			arch::map_page(frame->cr2, (u64)PHYSICAL(pmm::alloc()), arch::MFLAGS::KDATA | arch::MFLAGS::s2M);
			return;
		}
	}

	error("EXCEPTION %02x [%04llx] @ %02x:%p\n", (u32)frame->exc, frame->err, (u32)frame->cs, (void*)frame->rip);
	print_reg(frame, rax, rbx); printk("\n");
	print_reg(frame, rcx, rdx); printk("\n");
	print_reg(frame, rdi, rsi); printk("\n");
	print_reg(frame, rdx, cr2); printk("\n");
	print_reg(frame, rip, rfl); printk("\n");
	print_reg(frame, rsp, rbp); printk("\n");
	console::push_color(0xff710627);
	printk("Halting...\n");
	asm volatile ("movq %0, %%rsp" :: "r"(frame->rsp));
	asm volatile ("movq %0, %%rbp" :: "r"(frame->rbp));
	asm volatile ("cli");
	asm volatile ("hlt");
}

namespace arch::idt {
	idt_entry* idt;

	void idt_add_entry(u8 v, u64 isr, u8 type) {
		idt[v].dpl = 0;
		idt[v].gate_type = type;
		idt[v].ist = 0;
		idt[v].base0 =  isr & 0x000000000000ffff;
		idt[v].base1 = (isr & 0x00000000ffff0000) >> 16;
		idt[v].base2 = (isr & 0xffffffff00000000) >> 32;
		idt[v].present = 1;
		idt[v].ss = 0x08;
	}

	void init() {
		idt = (idt_entry*)pmm::alloc();
		memset(idt, 0, 0x1000);

		idt_add_entry(0x00, (u64)exc0,  0b1111);
		idt_add_entry(0x01, (u64)exc1,  0b1111);
		idt_add_entry(0x02, (u64)exc2,  0b1111);
		idt_add_entry(0x03, (u64)exc3,  0b1111);
		idt_add_entry(0x04, (u64)exc4,  0b1111);
		idt_add_entry(0x05, (u64)exc5,  0b1111);
		idt_add_entry(0x06, (u64)exc6,  0b1111);
		idt_add_entry(0x07, (u64)exc7,  0b1111);
		idt_add_entry(0x08, (u64)exc8,  0b1111);
		idt_add_entry(0x09, (u64)exc9,  0b1111);
		idt_add_entry(0x0a, (u64)exc10, 0b1111);
		idt_add_entry(0x0b, (u64)exc11, 0b1111);
		idt_add_entry(0x0c, (u64)exc12, 0b1111);
		idt_add_entry(0x0d, (u64)exc13, 0b1111);
		idt_add_entry(0x0e, (u64)exc14, 0b1111);
		idt_add_entry(0x0f, (u64)exc15, 0b1111);
		idt_add_entry(0x10, (u64)exc16, 0b1111);

		idtr i { 0x0fff, idt };
		asm volatile ("lidt %0" :: "m"(i));
	}
}
