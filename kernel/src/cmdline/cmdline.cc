#include <cmdline/cmdline.hh>
#include <mm/pmm.hh>
#include <mm/vmm.hh>

bool cmdline_active = false;

Arena<Command> cmds = {
	{ "help", [] (StringSplitIterator) {
		printk("Supported commands:\n");
		for (const auto& c : cmds) {
			printk("%s - %s\n", c.name, c.desc);
		}
	}, "Displays all commands" },

	{ "mem", [] (StringSplitIterator) {
		printlnk("PMM: %lld KiBs used, %lld KiBs free", bytes2kibs(pmm_usedmem), bytes2kibs(pmm_freemem));
		printlnk("VMM: %lld KiBs used, %lld KiBs free", bytes2kibs(g_vmm.usedmem), bytes2kibs(g_vmm.freemem));
	}, "Show memory usage" },

	{ "lookup", [] (StringSplitIterator args) {
		StringView a = *args;
		u64 virt = a.to_uint();
		u64 phys = paging_lookup(virt);
		printlnk("paging_lookup(%016llx) -> %016llx", virt, phys);
	}, "Translate virtual memory address to physical" },

	{ "xq", [] (StringSplitIterator args) {
		StringView a = *args;
		u64 virt = a.to_uint();
		u32 upper = virt >> 47;
		if (upper != 0xffff8 && upper != 0) {
			printlnk("Non-canonical address! Upper part of address must be 0xffff or 0x0000.");
			return;
		}
		if (paging_lookup(virt) == -1ull) {
			printlnk("Address is not mapped anywhere!");
			return;
		}
		printlnk("%p: 0x%016llx\n", (void*)virt, *(u64*)virt);
	}, "Read memory address (QWORD)" },

	{ "xl", [] (StringSplitIterator args) {
		StringView a = *args;
		u64 virt = a.to_uint();
		u32 upper = virt >> 47;
		if (upper != 0xffff8 && upper != 0) {
			printlnk("Non-canonical address! Upper part of address must be 0xffff or 0x0000.");
			return;
		}
		if (paging_lookup(virt) == -1ull) {
			printlnk("Address is not mapped anywhere!");
			return;
		}
		printlnk("%p: 0x%08x\n", (void*)virt, *(u32*)virt);
	}, "Read memory address (DWORD)" },

	{ "xw", [] (StringSplitIterator args) {
		StringView a = *args;
		u64 virt = a.to_uint();
		u32 upper = virt >> 47;
		if (upper != 0xffff8 && upper != 0) {
			printlnk("Non-canonical address! Upper part of address must be 0xffff or 0x0000.");
			return;
		}
		if (paging_lookup(virt) == -1ull) {
			printlnk("Address is not mapped anywhere!");
			return;
		}
		printlnk("%p: 0x%04x\n", (void*)virt, *(u16*)virt);
	}, "Read memory address (WORD)" },

	{ "xb", [] (StringSplitIterator args) {
		StringView a = *args;
		u64 virt = a.to_uint();
		u32 upper = virt >> 47;
		if (upper != 0xffff8 && upper != 0) {
			printlnk("Non-canonical address! Upper part of address must be 0xffff or 0x0000.");
			return;
		}
		if (paging_lookup(virt) == -1ull) {
			printlnk("Address is not mapped anywhere!");
			return;
		}
		printlnk("%p: 0x%02x\n", (void*)virt, *(u8*)virt);
	}, "Read memory address (BYTE)" },
};

static void prompt() {
	printk("debug > ");
}

void cmdline_activate() {
	cmdline_active = true;
	prompt();
}

void cmdline_deactivate() {
	cmdline_active = false;
}

void cmdline_evaluate_buffer(const char* buf, u64 sz) {
	if (!cmdline_active) return;

	auto cmd = StringSplitIterator(buf, ' ').begin();

	if (!sz) goto fine;

	for (const auto& c : cmds) {
		if ((*cmd) == c.name) {
			c.callback(++cmd);
			goto fine;
		}
	}

	printk("Nonsense\n");

fine:
	prompt();
}

cmd_handle_t cmdline_add_command(Command cmd) {
	return cmds.push_back(cmd);
}

void cmdline_remove_command(cmd_handle_t handle) {
	cmds.remove(handle);
}
