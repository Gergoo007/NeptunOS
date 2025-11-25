#include <arch/amd64/idt.hh>
#include <arch/amd64/paging.hh>
#include <arch/amd64/apic.hh>
#include <mm/vmm.hh>
#include <gfx/console.hh>

#define print_reg(st, reg, reg2) error("%s: %p  %s: %p", #reg, (void*)st->reg, #reg2, (void*)st->reg2)

volatile u64 tmr_counter = 0;

extern "C" void onInterrupt(arch_idt_frame_t* frame) {
	switch (frame->exc) {
		case 0xe: {
			if ((frame->cr2 & 0xffff900000000000) == 0xffff900000000000) {
				// this s2M flag cost me a piece of my soul
				map_page(frame->cr2, (u64)PHYSICAL(pmm_alloc()), KDATA | s2M);
				return;
			}
		}
		default: {
			error("EXCEPTION %02x [%04llx] @ %02x:%p", (u32)frame->exc, frame->err, (u32)frame->cs, (void*)frame->rip);
			print_reg(frame, rax, rbx);
			print_reg(frame, rcx, rdx);
			print_reg(frame, rdi, rsi);
			print_reg(frame, rdx, cr2);
			print_reg(frame, rip, rfl);
			print_reg(frame, rsp, rbp);
			con_push_color(0xff710627);
			printk("Halting...\n");
			asm volatile ("movq %0, %%rsp" :: "r"(frame->rsp));
			asm volatile ("movq %0, %%rbp" :: "r"(frame->rbp));
			asm volatile ("cli");
			asm volatile ("hlt");

			break;
		}

		case 0x40: {
			tmr_counter++;
			arch_lapic_eoi();
			break;
		}
	}
}

idt_entry_t* idt;

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

void arch_idt_init() {
	idt = (idt_entry_t*)pmm_alloc();
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

	idt_add_entry(0x40, (u64)exc64, 0b1111);

	idtr_t i { 0x0fff, idt };
	asm volatile ("lidt %0" :: "m"(i));
}
