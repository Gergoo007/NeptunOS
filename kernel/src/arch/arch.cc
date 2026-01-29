#include <arch/arch.hh>
#include <arch/limine.hh>

void* higherhalf;
framebuffer_t fbs[2];
struct limine_memmap_response* mmap;

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

void arch_read_boot_info() {
	limine_framebuffer_response* r = fb_req.response;

	if (!r)
		sprintk("Nincs framebuffer!\n\r");
	else
		sprintk("Van %d framebuffer\n\r", (u32)r->framebuffer_count);

	for (u32 i = 0; i < min(r->framebuffer_count, 2u); i++) {
		fbs[i].fb_addr = (u32*)r->framebuffers[i]->address;
		fbs[i].fb_bpp = r->framebuffers[i]->bpp;
		fbs[i].fb_width = r->framebuffers[i]->width;
		fbs[i].fb_height = r->framebuffers[i]->height;
	}

	higherhalf = (void*)hhdm_req.response->offset;
}

void sputs(const char* s) {
	while (*s) sputc(*(s++));
}
