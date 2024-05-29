#include <ps2/kbd.h>

// betű - 'a' = index [betű scancode-jához]
static const u8 kbd_set_3[] = {
	0x1C, 0x32, 0x21, 0x23, 0x24, 0x2B, 0x34, 0x33, 0x43, 0x3B, 0x42, 0x4B, 0x3A,
	0x31, 0x44, 0x4D, 0x15, 0x2D, 0x1B, 0x2C, 0x3C, 0x2A, 0x1D, 0x22, 0x35, 0x1A
};

void ps2_kbd_init() {
	clearint();

	// 'self-test' végzése
	// outb(0xaa, PS2_CMD);
	// u8 to = 1;
	// while (inb(PS2_STS) != 0x55 && to != 0) to++;

	// // letiltás míg konfigolom
	// outb(0xa7, PS2_CMD);
	// while (inb(PS2_STS) & 2);
	// outb(0xad, PS2_CMD);
	// while (inb(PS2_STS) & 2);

	// // adat eltávolítása
	// inb(PS2_DATA);

	// // adatok engedélyezése
	// outb(0xae, PS2_CMD);
	// while (inb(PS2_STS) & 2);
	// outb(0xa8, PS2_CMD);
	// while (inb(PS2_STS) & 2);

	// u8 typematic = 0;
	// // Repeat rate koxra
	// typematic |= 0b00000;
	// // Delay szintén max
	// typematic |= (0b00 << 5);

	// outb(PS2_KBD_REPEAT_DELAY, PS2_CMD);
	// while (inb(PS2_STS) & 2);
	// outb(typematic, PS2_DATA);
	// while (inb(PS2_STS) & 2);

	// outb(0xee, PS2_CMD);
	// while (inb(PS2_STS) & 2);
	// printk("resp %02x\n", inb(PS2_STS));

	// reset
	// outb(0xff, PS2_DATA);
	// while (inb(PS2_STS) & 2);

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

	outb(PS2_KBD_SCANCODE, PS2_DATA);
	while (inb(PS2_STS) & 2);

	// scancode készlet 3
	outb(PS2_KBD_SCANCODE, PS2_DATA);
	while (inb(PS2_STS) & 2);
	if (inb(PS2_DATA) != 0xfa) printk("bad\n");
	outb(3, PS2_DATA);
	while (inb(PS2_STS) & 2);

	printk("scancode %02x\n", inb(PS2_DATA));
	printk("scancode %02x\n", inb(PS2_DATA));

	setint();
}

char ps2_kbd_convert(u8 scancode) {
	switch (scancode) {
		case 0x66: return '\b';
		case 0x5a: return '\n';
		case 0x29: return ' ';
	}
	for (u8 i = 0; i < sizeof(kbd_set_3); i++)
		if (kbd_set_3[i] == scancode) return i + 'a';
	return 'f';
}
