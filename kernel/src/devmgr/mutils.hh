#pragma once

#include <devmgr/module.hh>

// Moduloknak utilok
#define MODULE_PROPS(namestr)					\
__attribute__((section(".modinfo"), used))		\
volatile modules::ModuleInfo _modinfo {			\
	.name = namestr,							\
};												\
