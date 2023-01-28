#pragma once

#include "attributes.h"

// 'asm' keyword is non-standard, only present in GNU C
#ifdef __STRICT_ANSI__
	#define asm __asm__
#endif

// Isn't useful when not using GUI
// Uses additional 4 MiBs of memory 
// 	+ relevant functions and variables
// #define USE_DOUBLE_BUFFERING

// Define this for verbose reports
#define DEBUG_REPORTS
