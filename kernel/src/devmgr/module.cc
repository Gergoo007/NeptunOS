#include <devmgr/module.hh>
#include <arch/limine.hh>
#include <util/ustar.hh>
#include <util/string.hh>
#include <arch/amd64/paging.hh>

__attribute__((section(".limine_requests"), used))
static volatile limine_module_request module_req {
	.id = LIMINE_MODULE_REQUEST,
	.revision = 1,
	.response = nullptr,

	.internal_module_count = 0,
	.internal_modules = nullptr,
};

namespace modules {
	void load() {
		
	}

	void register_all() {
		auto* r = module_req.response;
		if (!r) {
			warn("No modules found!\n");
			return;
		}

		report("num mods: %lld\n", r->module_count);
		for (u32 i = 0; i < r->module_count; i++) {
			if (!strcmp(r->modules[i]->path, "/modules.tar")) {
				arch::check_pages((u64)r->modules[i]->address, r->modules[i]->size);
				auto files = ustar::list(r->modules[i]->address, r->modules[i]->size);
				for (const auto& f : files) {
					report("found module: %s; %p\n", f.name, f.address);
				}
			}
		}
	}
}
