#include <scheduler/scheduler.hh>
#include <config.hh>
#include <arch/amd64/cpuid.hh>
#include <gfx/console.hh>

llist<task> sched_tasks;
// Current task id; gets incremented on task creation
u32 sched_current_id = 0;
task* kerneltask;
bool sched_running = false;

// Which process is each hardware thread running currently?
// idx is xAPIC ID, value is the task itself
static llist<task>::link_t* cpus[16];

void tesztfunc() {
	report("helo world from 1st context switch");
	pause();
}

extern "C" char sse_state[512];

void sched_start() {
	// Preparation: adding the kernel as process #0
	kerneltask = &sched_tasks.push_back(task {
		.state = {
			.cs = 0x08,
			.ss = 0x10,
		},
		.ssestate = (u8*)kmalloc(512),
		.id = 0,
		.parent = 0,
		.type = TaskType::PROCESS
	});

	cpus[cpuid_xapic_id()] = sched_tasks.first;

	report("halo halo ktask %p", kerneltask);
	sched_running = true;
}

void sched_add_thread(void (*entry)(void)) {
	task newt {
		.state = {
			.cs = 0x08,
			.ss = 0x10,
		},
		.ssestate = (u8*)kmalloc(512),
		.id = 1,
		.parent = 0,
		.type = TaskType::THREAD
	};
	newt.state.rip = (u64)entry;
	newt.state.rsp = (u64)kmalloc(0x10000) + 0x10000;
	newt.state.rbp = newt.state.rsp;
	newt.state.rfl = 0x200;

	report("ssestate for %d is %p", newt.id, newt.ssestate);

	sched_tasks.push_back(newt);
}

void sched_setrunning(bool otoole) {
	sched_running = otoole;
}

// Process state (cpu_state_t* in %rdi) needs to be saved into the interrupted tasks' task_t::state,
// and then the next task's state needs to be loaded in order to start execution

// =========!!!WARNING!!!=========
// EVERY KERNEL STATE CHANGE MADE BY THIS FUNCTION MUST BE ACCOUNTED FOR,
// AS WHEN THIS INTERRUPTS ANOTHER FUNCTION THAT MODIFIES THE SAME STATE,
// THE KERNEL STATE BECOMES CORRUPTED
// =========!!!WARNING!!!=========
void sched_tick(cpu_state_t* state) {
	if (!sched_running) return;

	auto*& ctask = cpus[cpuid_xapic_id()];

	// A state másolása a jelenlegi thread kontextusába
	// ctask->data.state = *state;
	memcpy(&ctask->data.state, state, sizeof(cpu_state_t));

	// TODO: ez miért korruptál?
	memcpy(ctask->data.ssestate, sse_state, 512); // FFFF9000003E8300
	vmm_check(ctask->data.ssestate);

	ctask = ctask->next;

	// A következő task kontextusának betöltése
	memcpy(sse_state, ctask->data.ssestate, 512);
	arch_cpu_state_load(&ctask->data.state);
}
