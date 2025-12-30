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
	// TODO: priority
};

// For now this is only a dumb round-robin
extern llist<task> sched_tasks;

void sched_start();
void sched_tick(cpu_state_t* state);
void sched_add_thread(void (*entry)(void));
void sched_setrunning(bool otoole);
