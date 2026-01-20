#pragma once

#include <types.hh>

struct AtaCmds {
	static constexpr u8 READ_DMA_EXT	= 0x25;
	static constexpr u8 WRITE_DMA_EXT	= 0x35;
	static constexpr u8 ZERO_EXT		= 0x44;
	static constexpr u8 IDENTIFY_DEVICE	= 0xec;
	static constexpr u8 SMART			= 0xb0;
};
