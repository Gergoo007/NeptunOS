#include "pic.h"

void pic_init(void) {
	u8 master_vec = 0x60;
	u8 slave_vec = 0x70;

	u8 mmask, smask;

	// Mask everything
	outb(0xff, MPIC_DATA);
	outb(0xff, SPIC_DATA);

	// idt_add_entry(ps2_kb_press_pic, 0x61);
	// idt_add_entry(pit_tick, 0x60);
	
	// Send init command along with CMD4 present bit
	outb(PIC_CMD1_INIT | PIC_CMD1_CMD4_PRESENT, MPIC_CMD);
	io_wait(2);
	outb(PIC_CMD1_INIT | PIC_CMD1_CMD4_PRESENT, SPIC_CMD);
	io_wait(2);

	// Then send the vector offsets
	outb(master_vec, MPIC_DATA);
	io_wait(2);
	outb(slave_vec, SPIC_DATA);
	io_wait(2);

	// There is another PIC at IRQ2
	outb(4, MPIC_DATA);
	io_wait(2);
	outb(2, SPIC_DATA);
	io_wait(2);

	// use 8086 mode instead of 8080
	outb(PIC_CMD4_8086, MPIC_DATA);
	io_wait(2);
	outb(PIC_CMD4_8086, SPIC_DATA);
	io_wait(2);

	// UnMask all IRQs
	mmask = 0xff;
	// mmask &= ~(1 << 1);
	mmask &= ~(1 << 0);
	smask = 0xff;
	outb(mmask, MPIC_DATA);
	// outb(0xff, MPIC_DATA);
	io_wait(2);
	outb(smask, SPIC_DATA);
	io_wait(2);
	
	io_wait(100);

	pit_init();
}
