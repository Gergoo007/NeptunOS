#pragma once

//typedef unsigned long long size_t;

typedef __SIZE_TYPE__ size_t;

#define NULL ((void*)0)

#ifndef __GNUC__
	#define offsetof(_struct, member) ((size_t)&(((_struct *)0)->member))
#else
	#define offsetof(_struct, member) __builtin_offsetof(_struct, member)
#endif
