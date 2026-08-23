#pragma once

#include <util/bits/int.hh>
#include <util/arena.hh>

struct Command {
	using fn = void (*)(StringSplitIterator);

	const char* name;
	fn callback;
	const char* desc;

	constexpr Command(): name(""), callback([](StringSplitIterator){}), desc("") { }
	constexpr Command(const char* n, fn cb, const char* d): name(n), callback(cb), desc(d) {  }
	constexpr ~Command() {  }
};

using cmd_handle_t = Arena<Command>::Handle;

extern bool cmdline_active;

void cmdline_evaluate_buffer(const char* buf, u64 sz);
void cmdline_activate();
void cmdline_deactivate();
cmd_handle_t cmdline_add_command(Command cmd);
void cmdline_remove_command(cmd_handle_t handle);
