#include <arch/arch.hh>
#include <arch/limine.hh>

Machine machine;
void* higherhalf;

__attribute__((used, section(".limine_requests")))
volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile limine_framebuffer_request fb_req {
	.id = LIMINE_FRAMEBUFFER_REQUEST,
	.revision = 0,
	.response = nullptr,
};

__attribute__((used, section(".limine_requests")))
static volatile limine_hhdm_request hhdm_req {
	.id = LIMINE_HHDM_REQUEST,
	.revision = 0,
	.response = nullptr,
};

__attribute__((used, section(".limine_requests")))
static volatile limine_stack_size_request stack_req {
	.id = LIMINE_STACK_SIZE_REQUEST,
	.revision = 0,
	.response = nullptr,
	.stack_size = mib2bytes(1),
};

__attribute__((used, section(".limine_requests_start")))
volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
volatile LIMINE_REQUESTS_END_MARKER;

namespace arch {
	void read_boot_info() {
		limine_framebuffer_response* r = fb_req.response;

		if (!r)
			sprintk("Nincs framebuffer!\n\r");
		else
			sprintk("Van %d framebuffer\n\r", r->framebuffer_count);

		for (u32 i = 0; i < min(r->framebuffer_count, 2); i++) {
			machine.fbs[i].fb_addr = (u32*)r->framebuffers[i]->address;
			machine.fbs[i].fb_bpp = r->framebuffers[i]->bpp;
			machine.fbs[i].fb_width = r->framebuffers[i]->width;
			machine.fbs[i].fb_height = r->framebuffers[i]->height;
		}

		higherhalf = (void*)hhdm_req.response->offset;
	}
}

void sputs(const char* s) {
	while (*s) sputc(*(s++));
}

void hlt() {
	arch::halt();
}
