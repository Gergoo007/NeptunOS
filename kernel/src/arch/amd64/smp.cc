#include <arch/amd64/smp.hh>
#include <arch/amd64/amd64.hh>
#include <arch/amd64/cpuid.hh>
#include <arch/limine.hh>
#include <util/async.hh>
#include <arch/amd64/apic.hh>
#include <scheduler/scheduler.hh>

__attribute__((used, section(".limine_requests")))
static volatile limine_mp_request smp_req = {
	.id = LIMINE_MP_REQUEST,
	.revision = 0,
	.response = nullptr,
	.flags = 0,
};

extern "C"
void ap_init(limine_mp_info* info) {
	arch_init(false);
	arch_late_init(false);

	for (auto& cpu : cpus)
		if (cpu.apic_id == info->lapic_id) cpu.up = true;

	pause();
}

void smp_init() {
	limine_mp_response* resp = smp_req.response;
	for (u32 i = 0; i < resp->cpu_count; i++)
		resp->cpus[i]->goto_address = ap_init;
}
