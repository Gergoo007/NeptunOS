#pragma once

#include <arch/arch.hh>
#include <util/storage.hh>

enum struct TaskType {
	PROCESS,
	THREAD
};

struct task {
	cpu_state_t state;
	u8* ssestate;
	u32 id;
	u32 parent;
	TaskType type;
	bool scheduled_for_deletion = false;
	// TODO: priority
};

// For now this is only a dumb round-robin
extern llist<task> sched_tasks;
extern llist<task>::link_t* sched_cpus[16];

void sched_start();
void sched_tick(cpu_state_t* state, bool force);
void sched_add_thread(void (*entry)());
void sched_setrunning(bool otoole);
