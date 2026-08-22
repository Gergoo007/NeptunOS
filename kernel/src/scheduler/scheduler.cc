#include <util/stacktrace.hh>
#include <scheduler/scheduler.hh>
#include <config.hh>
#include <arch/amd64/cpuid.hh>
#include <gfx/console.hh>
#include <mm/vmm.hh>

using task_elem = LinkedList<task>::Link;

MutexSimple sched_m;
LinkedList<task> sched_tasks;
bool sched_running = false;
task_elem* sched_cpus[16];

u32 sched_current_pid = 0;

void sched_dump() {
	report("Procs:");

	task_elem* l = sched_tasks.first,* cl;
	u32 i = 0;
	u32 current = sched_cpus[cpuid_xapic_id()]->data.id;
	do {
		report(
			"Task #%d: PID %d %s; parent %d; [%d %d]%s",
			i++, l->data.id, l->data.type == TaskType::PROCESS ? "process" : "thread",
			l->data.parent, l->prev->data.id, l->next->data.id, l->data.id == current ? " (CURRENT)" : ""
		);
		if (l->data.id == current) cl = l;
		l = l->next;
	} while (l != sched_tasks.first);
	(void)cl;
}

void sched_start() {
	assert(!sched_tasks.size);

	sched_m.lock();
	schedguard g;
	sched_tasks.push_front(task {
		.state = {
			.cs = 0x08,
			.ss = 0x10,
		},
		.ssestate = (u8*)kmalloc(512),
		.id = sched_current_pid++,
		.parent = 0,
		.type = TaskType::PROCESS
	});

	sched_cpus[cpuid_xapic_id()] = sched_tasks.first;
	sched_m.unlock();

	// Manuális interrupt hogy meg legyen a kontextus
	asm volatile ("int $0x41");
	sched_running = true;
}

void sched_exit_thread() {
	sched_m.lock();
	sched_setrunning(false);

	sched_cpus[cpuid_xapic_id()]->data.scheduled_for_deletion = true;

	sched_m.unlock();
	sched_setrunning(true);

	pause();
}

void sched_setrunning(bool otoole) { sched_running = otoole; }

void sched_tick(cpu_state_t* state, bool force, bool eoi) {
	if (!sched_running && !force) return;
	sched_m.lock();

	task_elem* ctask = sched_cpus[cpuid_xapic_id()];

	if (ctask->data.scheduled_for_deletion) {
		// Ez a task éppen most járt le, tehát
		// itt van egy esély a biztonságos kivételre a sched_tasks-ból
		task_elem* next = ctask->next;
		sched_tasks.remove(*ctask);
		sched_cpus[cpuid_xapic_id()] = next;
	} else {
		// Nincs megjelölve törlésre
		memcpy(&ctask->data.state, state, sizeof(cpu_state_t));
		memcpy(ctask->data.ssestate, sse_state, 512);
		sched_cpus[cpuid_xapic_id()] = ctask->next;
	}

	// A következő task kontextusának betöltése
	memcpy(sse_state, sched_cpus[cpuid_xapic_id()]->data.ssestate, 512);
	sched_m.unlock();
	if (eoi) arch_eoi();
	arch_cpu_state_load(&sched_cpus[cpuid_xapic_id()]->data.state);
}

void sched_add_user_process(u64 cr3, u64 stackptr, void (*entry)(int argc, char** argv)) {
	assert(sched_running);

	sched_m.lock();
	schedguard g;
	task newt {
		.state = {
			.cs = 0x20 | 3,
			.ss = 0x18 | 3,
		},
		.ssestate = (u8*)kmalloc(512),
		.id = sched_current_pid++,
		.parent = 0,
		.type = TaskType::PROCESS
	};
	newt.state.rip = (u64)entry;
	newt.state.rsp = stackptr;
	newt.state.rbp = stackptr;
	newt.state.rfl = 0x202;
	newt.state.rdi = 0;
	newt.state.cr3 = cr3;

	memcpy(newt.ssestate, sse_state, 512);

	sched_tasks.push_back(newt);
	sched_m.unlock();
}
