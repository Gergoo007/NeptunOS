#pragma once

#include "attributes.h"

// 'asm' keyword is non-standard, only present in GNU C
#ifdef __STRICT_ANSI__
	#define asm __asm__
#endif

// Useful on real hardware where FB is very slow
// #define USE_DOUBLE_BUFFERING

// Define this for verbose reports
#define DEBUG_REPORTS

// Define this for serial logging
#define SERIAL_DEBUG
