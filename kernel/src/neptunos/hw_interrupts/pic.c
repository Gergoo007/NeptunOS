#include <neptunos/hw_interrupts/pic.h>

void remap_pic(uint8_t offset1, uint8_t offset2) {
	uint8_t a1, a2;
 
	a1 = inb(PIC_M_DATA);								// save masks
	a2 = inb(PIC_S_DATA);
 
	outb(ICW1_INIT | ICW1_ICW4, PIC_M_COMMAND);// starts the initialization sequence (in cascade mode)
	outb(ICW1_INIT | ICW1_ICW4, PIC_S_COMMAND);

	outb(offset1, PIC_M_DATA);					// ICW2: Master PIC vector offset
	outb(offset2, PIC_S_DATA);					// ICW2: Slave PIC vector offset
	outb(4, PIC_M_DATA);						// ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	outb(2, PIC_S_DATA);						// ICW3: tell Slave PIC its cascade identity (0000 0010)

	outb(ICW4_8086, PIC_M_DATA);
	outb(ICW4_8086, PIC_S_DATA);

	outb(a1, PIC_M_DATA);						// restore saved masks.
	outb(a2, PIC_S_DATA);
}
