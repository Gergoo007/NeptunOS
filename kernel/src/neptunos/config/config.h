#pragma once

#include "attributes.h"

// 'asm' keyword is only valid in GNU C
#define asm __asm__

// Isn't useful when not using GUI
// Uses additional 4 MiBs of memory 
// 	+ relevant functions and variables
// #define USE_DOUBLE_BUFFERING
