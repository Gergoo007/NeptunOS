#pragma once

#include <types.hh>

struct i8042Cmds {
	static constexpr u32 READ_CFG = 0x20;
	static constexpr u32 WRITE_CFG = 0x60;

	static constexpr u32 SELF_TEST = 0xaa;

	static constexpr u32 ENABLE_FIRST = 0xae;
	static constexpr u32 DISABLE_FIRST = 0xad;
	static constexpr u32 ENABLE_SECOND = 0xa8;
	static constexpr u32 DISABLE_SECOND = 0xa7;

	static constexpr u32 TEST_FIRST = 0xab;
	static constexpr u32 TEST_SECOND = 0xa9;
};

punion i8042_sts {
	pstruct {
		u8 output_buf_full : 1;
		u8 input_buf_full : 1;
		u8 : 1;
		u8 data_is_for_device : 1;
		u8 : 4;
	};
	u8 raw;
};

struct Ps2Cmds {
	static constexpr u32 SET_LED = 0xed;
	static constexpr u32 ECHO = 0xee;
	static constexpr u32 SCANCODESET = 0xf0;
	static constexpr u32 IDENTIFY = 0xf2;
	static constexpr u32 RATE_DELAY = 0xf3;
	static constexpr u32 ENABLE = 0xf4;
	static constexpr u32 DISABLE_SCANNING = 0xf5;
	static constexpr u32 SET_DEFAULTS = 0xf6;

	static constexpr u32 RESET = 0xff;
};

punion config_byte {
	pstruct {
		u8 irq_kb : 1;					// First PS/2 port interrupt (1 = enabled, 0 = disabled)
		u8 irq_mouse : 1;				// Second PS/2 port interrupt (1 = enabled, 0 = disabled, only if 2 PS/2 ports supported)
		u8 alwaysone : 1;				// System Flag (1 = system passed POST, 0 = your OS shouldn't be running)
		u8 maybezero : 1;				// Should be zero
		u8 clock_kb : 1;				// First PS/2 port clock (1 = disabled, 0 = enabled)
		u8 clock_mouse : 1;				// Second PS/2 port clock (1 = disabled, 0 = enabled, only if 2 PS/2 ports supported)
		u8 first_port_translation : 1;	// First PS/2 port translation (1 = enabled, 0 = disabled)
		u8 alwayszero : 1;				// Must be zero
	};
	u8 raw;
};

// // 'A'..='Z'
// static constexpr u8 set2_alphabet[] = {
// 	0x1c, 0x32, 0x21, 0x23, 0x24, 0x2b, 0x34, 0x33, 0x43, 0x3b, 0x42, 0x4b, 0x3a, 0x31, 0x44, 0x4d, 0x15, 0x2d, 0x1b, 0x2c, 0x3c, 0x2a, 0x1d, 0x22, 0x35, 0x1a,
// };

// // '0'..='9'
// static constexpr u8 set2_numbers[] = {
// 	0x45, 0x16, 0x1e, 0x26, 0x25, 0x2e, 0x36, 0x3d, 0x3e, 0x46
// };

constexpr char scancode_set2_to_ascii[256] = {
    /* 0x00 */ 0,
    /* 0x01 */ 0,
    /* 0x02 */ 0,
    /* 0x03 */ 0,
    /* 0x04 */ 0,
    /* 0x05 */ 0,
    /* 0x06 */ 0,
    /* 0x07 */ 0,
    /* 0x08 */ 0,
    /* 0x09 */ 0,
    /* 0x0A */ 0,
    /* 0x0B */ 0,
    /* 0x0C */ 0,
    /* 0x0D */ '\t',     // tab
    /* 0x0E */ '`',      // backtick
    /* 0x0F */ 0,

    /* 0x10 */ 0,
    /* 0x11 */ 0,
    /* 0x12 */ 0,
    /* 0x13 */ 0,
    /* 0x14 */ 0,
    /* 0x15 */ 'q',
    /* 0x16 */ '1',
    /* 0x17 */ 0,
    /* 0x18 */ 0,
    /* 0x19 */ 0,
    /* 0x1A */ 'z',
    /* 0x1B */ 's',
    /* 0x1C */ 'a',
    /* 0x1D */ 'w',
    /* 0x1E */ '2',
    /* 0x1F */ 0,

    /* 0x20 */ 0,
    /* 0x21 */ 'c',
    /* 0x22 */ 'x',
    /* 0x23 */ 'd',
    /* 0x24 */ 'e',
    /* 0x25 */ '4',
    /* 0x26 */ '3',
    /* 0x27 */ 0,
    /* 0x28 */ 0,
    /* 0x29 */ ' ',
    /* 0x2A */ 'v',
    /* 0x2B */ 'f',
    /* 0x2C */ 't',
    /* 0x2D */ 'r',
    /* 0x2E */ '5',
    /* 0x2F */ 0,

    /* 0x30 */ 0,
    /* 0x31 */ 'n',
    /* 0x32 */ 'b',
    /* 0x33 */ 'h',
    /* 0x34 */ 'g',
    /* 0x35 */ 'y',
    /* 0x36 */ '6',
    /* 0x37 */ 0,
    /* 0x38 */ 0,
    /* 0x39 */ 0,
    /* 0x3A */ 'm',
    /* 0x3B */ 'j',
    /* 0x3C */ 'u',
    /* 0x3D */ '7',
    /* 0x3E */ '8',
    /* 0x3F */ 0,

    /* 0x40 */ 0,
    /* 0x41 */ ',',
    /* 0x42 */ 'k',
    /* 0x43 */ 'i',
    /* 0x44 */ 'o',
    /* 0x45 */ '0',
    /* 0x46 */ '9',
    /* 0x47 */ 0,
    /* 0x48 */ 0,
    /* 0x49 */ '.',
    /* 0x4A */ '/',
    /* 0x4B */ 'l',
    /* 0x4C */ ';',
    /* 0x4D */ 'p',
    /* 0x4E */ '-',
    /* 0x4F */ 0,

    /* 0x50 */ 0,
    /* 0x51 */ 0,
    /* 0x52 */ '\'',
    /* 0x53 */ 0,
    /* 0x54 */ '[',
    /* 0x55 */ '=',
    /* 0x56 */ 0,
    /* 0x57 */ 0,
    /* 0x58 */ 0,
    /* 0x59 */ 0,
    /* 0x5A */ '\n',    // enter
    /* 0x5B */ ']',
    /* 0x5C */ 0,
    /* 0x5D */ '\\',
    /* 0x5E */ 0,
    /* 0x5F */ 0,
};
