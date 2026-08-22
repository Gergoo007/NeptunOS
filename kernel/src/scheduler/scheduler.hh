#pragma once

#include <arch/arch.hh>
#include <arch/amd64/paging.hh>
#include <util/storage.hh>
#include <mm/vmm.hh>

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

struct schedguard {
	schedguard() { sched_setrunning(false); }
	~schedguard() { sched_setrunning(true); }
};

// For now this is only a dumb round-robin
extern LinkedList<task> sched_tasks;
extern LinkedList<task>::Link* sched_cpus[16];

void sched_start();
void sched_tick(cpu_state_t* state, bool force, bool eoi);
void sched_exit_thread();
void sched_setrunning(bool otoole);

extern MutexSimple sched_m;
extern u32 sched_current_pid;
extern "C" u8 sse_state[512];

template<typename T>
void sched_add_thread(T&& fun) {
	sched_m.lock();
	schedguard g;
	task newt {
		.state = {
			.cs = 0x08,
			.ss = 0x10,
		},
		.ssestate = (u8*)kmalloc(512),
		.id = sched_current_pid++,
		.parent = 0,
		.type = TaskType::THREAD
	};
	newt.state.rip = (u64)(void*)(&T::operator());
	newt.state.rsp = (u64)kmalloc(0x10000) + 0x8000;
	newt.state.rbp = newt.state.rsp;
	newt.state.rfl = 0x202;
	newt.state.rdi = (u64)(void*)&fun;
	newt.state.cr3 = paging_lookup(pml4);

	memcpy(newt.ssestate, sse_state, 512);

	// Thread exit return cím pusholása
	newt.state.rsp -= 8;
	*(u64*)newt.state.rsp = (u64)sched_exit_thread;

	sched_tasks.push_back(newt);
	sched_m.unlock();
}

struct page_table_t;
void sched_add_user_process(u64 cr3, u64 stackptr, void (*entry)(int argc, char** argv));
