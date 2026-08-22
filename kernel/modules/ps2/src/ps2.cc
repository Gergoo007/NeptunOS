#include <gfx/console.hh>
#include <devmgr/module.hh>
#include <arch/amd64/io.hh>
#include <arch/arch.hh>
#include <devmgr/input/input.hh>

#include "ps2.hh"

__attribute__((section(".modinfo"), used))
volatile constexpr module_metadata_t _modinfo {
	.name = "ps2",
	.triggertype = ModuleTriggerTypes::FADT_LEGACY_I8042,
	.trigger = {  }
};

constexpr u16 CMD = 0x64;
constexpr u16 DATA = 0x60;

static i8042_sts getsts() {
	i8042_sts ret;
	ret.raw = inb(CMD);
	return ret;
}

[[nodiscard]]
static bool poll_data_available() {
	u32 tries = 10;
	while (!getsts().output_buf_full && tries) {
		arch_sleep(10, true);
		tries--;
	}
	return tries;
}

static void poll_standby() {
	u32 tries = 10;
	while (getsts().input_buf_full && tries) {
		arch_sleep(10, true);
		tries--;
	}
	if (!tries)
		fatal("Input buffer is not getting cleared!");
}

static u8 read() {
	if (poll_data_available())
		return inb(DATA);
	else
		return 0;
}

static void write(u8 data) {
	poll_standby();
	outb(DATA, data);
}

static void command(u8 cmd) {
	poll_standby();
	outb(CMD, cmd);
}

bool released = false;
void handlekb(cpu_state_t* frame) {
// 	u8 code = read();
// 	if (code == 0xf0) {
// 		released = true;
// 		goto end;
// 	}
// 	if (code > 0x5f) goto end;
// 	if (released) {
// 		released = false;
// 		goto end;
// 	}

// 	printk("%c", scancode_set2_to_ascii[code]);

// end:
// 	arch_eoi();

	u8 code = read();
	if (code == 0xf0) {
		released = true;
		goto end;
	}
	if (code > 0x5f) goto end;
	if (released) {
		released = false;
		// kbd_release((ScanCode)scancode_set2_to_ascii[code]);
	} else {
		// kbd_press((ScanCode)scancode_set2_to_ascii[code]);
		printk("%c", scancode_set2_to_ascii[code]);
	}

end:
	arch_eoi();
}

void handlemouse(cpu_state_t* frame) {
	// TODO

	read();
	arch_eoi();
}

extern "C" void mod_main() {
	// Billentyűzet/egér kikapcsolása
	command(i8042Cmds::DISABLE_FIRST);
	command(i8042Cmds::DISABLE_SECOND);

	// Bemeneti puffer kiűrítése
	inb(DATA);

	// Config byte
	config_byte cfg;
	command(i8042Cmds::READ_CFG);
	cfg.raw = read();
	cfg.clock_kb = 0;
	cfg.irq_kb = 0;
	cfg.first_port_translation = 0;
	command(i8042Cmds::WRITE_CFG);
	write(cfg.raw);

	// Self test
	command(i8042Cmds::SELF_TEST);
	if (read() != 0x55) {
		error("PS/2 controller not working (self-test failed)!");
		return;
	}

	command(i8042Cmds::WRITE_CFG);
	write(cfg.raw);

	command(i8042Cmds::ENABLE_SECOND);

	command(i8042Cmds::READ_CFG);
	cfg.raw = read();

	if (cfg.clock_mouse) {
		report("PS/2 port #2 is present");
		command(i8042Cmds::DISABLE_SECOND);

		command(i8042Cmds::READ_CFG);
		cfg.raw = read();
	}

	bool port1 = true, port2 = true;

	command(i8042Cmds::TEST_FIRST);
	u8 resp = read();
	if (resp != 0x00) {
		warn("PS/2 device on first port replied with '%02x'", resp);
		port1 = false;
	}

	command(i8042Cmds::TEST_SECOND);
	resp = read();
	if (resp != 0x00) {
		warn("PS/2 device on second port replied with '%02x'", resp);
		port2 = false;
	}

	if (port1) {
		report("Enabling port #1");
		command(i8042Cmds::ENABLE_FIRST);

		command(i8042Cmds::READ_CFG);
		cfg.raw = read();

		cfg.clock_kb = 0;
		cfg.irq_kb = 1;
		cfg.first_port_translation = 0;
		// cfg.first_port_translation = 0;

		command(i8042Cmds::WRITE_CFG);
		write(cfg.raw);
	}

	if (port2) {
		report("Enabling port #2");
		command(i8042Cmds::ENABLE_SECOND);

		command(i8042Cmds::READ_CFG);
		cfg.raw = read();

		cfg.clock_mouse = 0;
		cfg.irq_mouse = 1;
		command(i8042Cmds::WRITE_CFG);
		write(cfg.raw);
	}

	arch_assign_irq(1, handlekb);
	arch_assign_irq(12, handlemouse);
}
