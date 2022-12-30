#include <neptunos/hw_interrupts/pic.h>

void remap_pic(uint8_t offset1, uint8_t offset2) {
	uint8_t a1, a2;
 
	a1 = in8(PIC_M_DATA);								// save masks
	a2 = in8(PIC_S_DATA);
 
	_out8(PIC_M_COMMAND, ICW1_INIT | ICW1_ICW4);// starts the initialization sequence (in cascade mode)
	_out8(PIC_S_COMMAND, ICW1_INIT | ICW1_ICW4);
	_out8(PIC_M_DATA, offset1);					// ICW2: Master PIC vector offset
	_out8(PIC_S_DATA, offset2);					// ICW2: Slave PIC vector offset
	_out8(PIC_M_DATA, 4);						// ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	_out8(PIC_S_DATA, 2);						// ICW3: tell Slave PIC its cascade identity (0000 0010)

	_out8(PIC_M_DATA, ICW4_8086);
	_out8(PIC_S_DATA, ICW4_8086);

	_out8(PIC_M_DATA, a1);						// restore saved masks.
	_out8(PIC_S_DATA, a2);
}
