#include <arch/amd64/idt.hh>
#include <arch/amd64/paging.hh>
#include <arch/amd64/apic.hh>
#include <arch/amd64/cpuid.hh>
#include <mm/vmm.hh>
#include <gfx/console.hh>
#include <scheduler/scheduler.hh>
#include <util/stacktrace.hh>
#include <devmgr/input/input.hh>

#define print_reg(st, reg, reg2) error("%s: %p  %s: %p", #reg, (void*)st->reg, #reg2, (void*)st->reg2)

atomic<u64> tmr_counter = 0;

void (*handler[256])(cpu_state_amd64_t* frame);
bitmap_t handlers;

u32 servicing = 0;

extern "C" void onInterrupt(cpu_state_amd64_t* frame) {
	if (servicing)
		fatal("ISR for %02x already in progress!?", servicing);
	servicing = frame->exc;
	switch (frame->exc) {
		case 0xe: {
			if ((frame->cr2 >> 40) == 0xffff90) {
				// this s2M flag cost me a piece of my soul
				map_page(frame->cr2, (u64)PHYSICAL(pmm_alloc()), KDATA | s2M);
				arch_sti();
				break;
			}
		}
		// fall through
		default: {
			arch_ioapic_disable_all();
			error("EXCEPTION %02x [%04llx] @ %02x:%p @ CPU %d THR %d", (u32)frame->exc, frame->err, (u32)frame->cs, (void*)frame->rip, cpuid_xapic_id(), sched_cpus[cpuid_xapic_id()] ? sched_cpus[cpuid_xapic_id()]->data.id : -1);
			print_reg(frame, rax, rbx);
			print_reg(frame, rcx, rdx);
			print_reg(frame, rdi, rsi);
			print_reg(frame, rdx, cr2);
			print_reg(frame, rip, rfl);
			print_reg(frame, rsp, rbp);
			if (*(u16*)frame->rip == 0xcf48) {
				error("IRETQ detected! Here's the relevant stack frame:");
				error("%%rip   : %p", *(u64**)(frame->rsp + 0));
				error("%%cs    : %04x", *(u16*)(frame->rsp + 8));
				error("%%rflags: %p", *(u64**)(frame->rsp + 16));
				error("%%rsp   : %p", *(u64**)(frame->rsp + 24));
				error("%%ss    : %04x", *(u16*)(frame->rsp + 32));
			}
			con_push_color(con_colors[3]);
			stacktrace(frame->rsp);
			con_push_color(con_colors[4]);
			printk("Halting...\n");
			asm volatile ("movq %0, %%rsp" :: "r"(frame->rsp));
			asm volatile ("movq %0, %%rbp" :: "r"(frame->rbp));
			pause();

			break;
		}

		case 0x40: {
			tmr_counter++;
			
			if (tmr_counter % VMM_REDZONE_CHECK_PERIOD == 0)
				g_vmm.check_all();

			for (const auto& t : timer_tasks) {
				if (tmr_counter % t.period == 0) t.routine();
			}

			if (tmr_counter % kbd_repeat == 0 && tmr_counter > kbd_lastpressed_time + kbd_delay) {
				kbd_onrepeat();
			}

			if (tmr_counter % SCHED_QUANTUM == 0) {
				servicing = 0;
				sched_tick(frame, false, true);
			}

			arch_lapic_eoi();
			break;
		}

		case 0x41: {
			servicing = 0;
			sched_tick(frame, true, false);
			break;
		}

		case 0x42: {
			error("SMI event!");
			arch_lapic_eoi();
			break;
		}

		case 0x60: {
			// warn("MSI FROM AHCI!!");
			arch_lapic_eoi();
			break;
		}
	}
	servicing = 0;
}

void idt_add_entry(idt_entry_t* idt, u8 v, u64 isr, u8 type) {
	idt[v].dpl = 0;
	idt[v].gate_type = type;
	idt[v].ist = 1;
	idt[v].base0 =  isr & 0x000000000000ffff;
	idt[v].base1 = (isr & 0x00000000ffff0000) >> 16;
	idt[v].base2 = (isr & 0xffffffff00000000) >> 32;
	idt[v].present = 1;
	idt[v].ss = 0x08;
}

u8 idt_allocate_vector(void (*handlerfun)(cpu_state_amd64_t* frame)) {
	for (u32 i = 0x21; i < 256; i++) {
		if (handler[i] == onInterrupt) {
			// Ez a vektor szabad
			handler[i] = handlerfun;
			return i;
		}
	}

	fatal("No free interrupt vector left");
	return -1;
}

void idt_deallocate_vector(u8 vector) {
	assert(handler[vector] != onInterrupt);
	handler[vector] = onInterrupt;
}

#define EXPAND2(n) exc##n
#define ADD_ENTRY(n) idt_add_entry(idt, n, (u64)EXPAND2(n),  0b1111);

void arch_idt_init() {
	idt_entry_t* idt = (idt_entry_t*)wm_alloc(0x1000);

	ADD_ENTRY(  0); ADD_ENTRY(  1); ADD_ENTRY(  2); ADD_ENTRY(  3);
    ADD_ENTRY(  4); ADD_ENTRY(  5); ADD_ENTRY(  6); ADD_ENTRY(  7);
    ADD_ENTRY(  8); ADD_ENTRY(  9); ADD_ENTRY( 10); ADD_ENTRY( 11);
    ADD_ENTRY( 12); ADD_ENTRY( 13); ADD_ENTRY( 14); ADD_ENTRY( 15);
    ADD_ENTRY( 16); ADD_ENTRY( 17); ADD_ENTRY( 18); ADD_ENTRY( 19);
    ADD_ENTRY( 20); ADD_ENTRY( 21); ADD_ENTRY( 22); ADD_ENTRY( 23);
    ADD_ENTRY( 24); ADD_ENTRY( 25); ADD_ENTRY( 26); ADD_ENTRY( 27);
    ADD_ENTRY( 28); ADD_ENTRY( 29); ADD_ENTRY( 30); ADD_ENTRY( 31);
    ADD_ENTRY( 32); ADD_ENTRY( 33); ADD_ENTRY( 34); ADD_ENTRY( 35);
    ADD_ENTRY( 36); ADD_ENTRY( 37); ADD_ENTRY( 38); ADD_ENTRY( 39);
    ADD_ENTRY( 40); ADD_ENTRY( 41); ADD_ENTRY( 42); ADD_ENTRY( 43);
    ADD_ENTRY( 44); ADD_ENTRY( 45); ADD_ENTRY( 46); ADD_ENTRY( 47);
    ADD_ENTRY( 48); ADD_ENTRY( 49); ADD_ENTRY( 50); ADD_ENTRY( 51);
    ADD_ENTRY( 52); ADD_ENTRY( 53); ADD_ENTRY( 54); ADD_ENTRY( 55);
    ADD_ENTRY( 56); ADD_ENTRY( 57); ADD_ENTRY( 58); ADD_ENTRY( 59);
    ADD_ENTRY( 60); ADD_ENTRY( 61); ADD_ENTRY( 62); ADD_ENTRY( 63);
    ADD_ENTRY( 64); ADD_ENTRY( 65); ADD_ENTRY( 66); ADD_ENTRY( 67);
    ADD_ENTRY( 68); ADD_ENTRY( 69); ADD_ENTRY( 70); ADD_ENTRY( 71);
    ADD_ENTRY( 72); ADD_ENTRY( 73); ADD_ENTRY( 74); ADD_ENTRY( 75);
    ADD_ENTRY( 76); ADD_ENTRY( 77); ADD_ENTRY( 78); ADD_ENTRY( 79);
    ADD_ENTRY( 80); ADD_ENTRY( 81); ADD_ENTRY( 82); ADD_ENTRY( 83);
    ADD_ENTRY( 84); ADD_ENTRY( 85); ADD_ENTRY( 86); ADD_ENTRY( 87);
    ADD_ENTRY( 88); ADD_ENTRY( 89); ADD_ENTRY( 90); ADD_ENTRY( 91);
    ADD_ENTRY( 92); ADD_ENTRY( 93); ADD_ENTRY( 94); ADD_ENTRY( 95);
    ADD_ENTRY( 96); ADD_ENTRY( 97); ADD_ENTRY( 98); ADD_ENTRY( 99);
    ADD_ENTRY(100); ADD_ENTRY(101); ADD_ENTRY(102); ADD_ENTRY(103);
    ADD_ENTRY(104); ADD_ENTRY(105); ADD_ENTRY(106); ADD_ENTRY(107);
    ADD_ENTRY(108); ADD_ENTRY(109); ADD_ENTRY(110); ADD_ENTRY(111);
    ADD_ENTRY(112); ADD_ENTRY(113); ADD_ENTRY(114); ADD_ENTRY(115);
    ADD_ENTRY(116); ADD_ENTRY(117); ADD_ENTRY(118); ADD_ENTRY(119);
    ADD_ENTRY(120); ADD_ENTRY(121); ADD_ENTRY(122); ADD_ENTRY(123);
    ADD_ENTRY(124); ADD_ENTRY(125); ADD_ENTRY(126); ADD_ENTRY(127);
    ADD_ENTRY(128); ADD_ENTRY(129); ADD_ENTRY(130); ADD_ENTRY(131);
    ADD_ENTRY(132); ADD_ENTRY(133); ADD_ENTRY(134); ADD_ENTRY(135);
    ADD_ENTRY(136); ADD_ENTRY(137); ADD_ENTRY(138); ADD_ENTRY(139);
    ADD_ENTRY(140); ADD_ENTRY(141); ADD_ENTRY(142); ADD_ENTRY(143);
    ADD_ENTRY(144); ADD_ENTRY(145); ADD_ENTRY(146); ADD_ENTRY(147);
    ADD_ENTRY(148); ADD_ENTRY(149); ADD_ENTRY(150); ADD_ENTRY(151);
    ADD_ENTRY(152); ADD_ENTRY(153); ADD_ENTRY(154); ADD_ENTRY(155);
    ADD_ENTRY(156); ADD_ENTRY(157); ADD_ENTRY(158); ADD_ENTRY(159);
    ADD_ENTRY(160); ADD_ENTRY(161); ADD_ENTRY(162); ADD_ENTRY(163);
    ADD_ENTRY(164); ADD_ENTRY(165); ADD_ENTRY(166); ADD_ENTRY(167);
    ADD_ENTRY(168); ADD_ENTRY(169); ADD_ENTRY(170); ADD_ENTRY(171);
    ADD_ENTRY(172); ADD_ENTRY(173); ADD_ENTRY(174); ADD_ENTRY(175);
    ADD_ENTRY(176); ADD_ENTRY(177); ADD_ENTRY(178); ADD_ENTRY(179);
    ADD_ENTRY(180); ADD_ENTRY(181); ADD_ENTRY(182); ADD_ENTRY(183);
    ADD_ENTRY(184); ADD_ENTRY(185); ADD_ENTRY(186); ADD_ENTRY(187);
    ADD_ENTRY(188); ADD_ENTRY(189); ADD_ENTRY(190); ADD_ENTRY(191);
    ADD_ENTRY(192); ADD_ENTRY(193); ADD_ENTRY(194); ADD_ENTRY(195);
	ADD_ENTRY(196); ADD_ENTRY(197); ADD_ENTRY(198); ADD_ENTRY(199);
	ADD_ENTRY(200); ADD_ENTRY(201); ADD_ENTRY(202); ADD_ENTRY(203);
    ADD_ENTRY(204); ADD_ENTRY(205); ADD_ENTRY(206); ADD_ENTRY(207);
    ADD_ENTRY(208); ADD_ENTRY(209); ADD_ENTRY(210); ADD_ENTRY(211);
    ADD_ENTRY(212); ADD_ENTRY(213); ADD_ENTRY(214); ADD_ENTRY(215);
    ADD_ENTRY(216); ADD_ENTRY(217); ADD_ENTRY(218); ADD_ENTRY(219);
    ADD_ENTRY(220); ADD_ENTRY(221); ADD_ENTRY(222); ADD_ENTRY(223);
    ADD_ENTRY(224); ADD_ENTRY(225); ADD_ENTRY(226); ADD_ENTRY(227);
    ADD_ENTRY(228); ADD_ENTRY(229); ADD_ENTRY(230); ADD_ENTRY(231);
    ADD_ENTRY(232); ADD_ENTRY(233); ADD_ENTRY(234); ADD_ENTRY(235);
    ADD_ENTRY(236); ADD_ENTRY(237); ADD_ENTRY(238); ADD_ENTRY(239);
    ADD_ENTRY(240); ADD_ENTRY(241); ADD_ENTRY(242); ADD_ENTRY(243);
    ADD_ENTRY(244); ADD_ENTRY(245); ADD_ENTRY(246); ADD_ENTRY(247);
    ADD_ENTRY(248); ADD_ENTRY(249); ADD_ENTRY(250); ADD_ENTRY(251);
    ADD_ENTRY(252); ADD_ENTRY(253); ADD_ENTRY(254); ADD_ENTRY(255);

	for (u32 i = 0; i < 256; i++)
		handler[i] = onInterrupt;

	idtr_t i { 0x0fff, idt };
	asm volatile ("lidt %0" :: "m"(i));
}
