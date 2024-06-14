#include <ps2/kbd.h>

#include <apic/apic.h>

// betű - 'a' = index [betű scancode-jához]
static const u8 kbd_set_3[] = {
	// Betűk
	0x1C, 0x32, 0x21, 0x23, 0x24, 0x2B, 0x34, 0x33, 0x43, 0x3B, 0x42, 0x4B, 0x3A,
	0x31, 0x44, 0x4D, 0x15, 0x2D, 0x1B, 0x2C, 0x3C, 0x2A, 0x1D, 0x22, 0x35, 0x1A,

	// Számok
	// In comment: english 0 scancode (key right of 9)
	0x0e, /* 0x45, */ 0x16, 0x1e, 0x26, 0x25, 0x2e, 0x36, 0x3d, 0x3e, 0x46,
};

void ps2_kbd_init() {
	clearint();

	inb(PS2_DATA);

	// translation kikapcs
	// bit 6 a config byte-ban
	outb(0x20, PS2_CMD);
	while (inb(PS2_STS) & 2);
	u8 config = inb(PS2_DATA);
	config &= ~(1 << 6);

	outb(0x60, PS2_CMD);
	while (inb(PS2_STS) & 2);
	outb(config, PS2_DATA);
	while (inb(PS2_STS) & 2);

	// scancode készlet 3
	outb(PS2_KBD_SCANCODE, PS2_DATA);
	while (inb(PS2_STS) & 2);
	if (inb(PS2_DATA) != 0xfa) error("Varatlan PS/2 valasz!");
	outb(3, PS2_DATA);
	while (inb(PS2_STS) & 2);

	// Jelenlegi készlet
	u8 current_set;
	outb(PS2_KBD_SCANCODE, PS2_DATA);
	while (inb(PS2_STS) & 2);
	if (inb(PS2_DATA) != 0xfa) error("Varatlan PS/2 valasz!");
	outb(0, PS2_DATA);
	while (inb(PS2_STS) & 2);
	if (inb(PS2_DATA) != 0xfa) error("Varatlan PS/2 valasz!");
	current_set = inb(PS2_DATA);

	if (current_set != 0x3f && current_set != 0x03)
		error("PS/2 scancode keszlet 3 nem elerheto! jelenlegi: %02x\n", current_set);

	inb(PS2_DATA);

	setint();

	ioapic_set_mask(irq_to_gsi(IRQ_KB), 0);
}

char ps2_kbd_convert(u8 scancode) {
	switch (scancode) {
		case 0x66: return '\b';
		case 0x5a: return '\n';
		case 0x29: return ' ';
	}

	for (u8 i = 0; i < sizeof(kbd_set_3); i++) {
		if (kbd_set_3[i] == scancode) {
			if (i > 25)
				return i - 26 + '0';
			else
				return i + 'a';
		}
	}
	sprintk("Kezeletlen scancode: %02x\n\r", scancode);
	return '*';
}
