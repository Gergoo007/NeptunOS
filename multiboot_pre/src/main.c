#include "multiboot2.h"
#include "elf.h"
#include "pmm/mmap.h"
#include "pmm/pmm.h"
#include "serial.h"
#include "paging/paging.h"

extern void* _binary_out_kernel_start;
extern u64 _binary_out_kernel_size;
extern void* _binary_out_kernel_end;
extern void* p4_table;
multiboot_tag_framebuffer_t* fb_tag;
void* fb_base;
u64 mb_hdr_length = 0;
multiboot_hdr_t* mbi;

tag_align __attribute__((section(".multiboot"))) multiboot_hdr_t header = {
	// Magic
	0xE85250D6,
	// Arch
	0,
	// Hdr length
	sizeof(multiboot_hdr_t),
	// Checksum (magic fields + checksum = 0)
	(u32)(0-0xE85250D6-0-sizeof(multiboot_hdr_t)),

	.fb_tag = {
		5,		// Type
		0,		// Flags
		20,		// Size
		1280,	// Width
		720,	// Height
		32,		// BPP (bits, not bytes)
	},

	.end_tag = {
		0,
		0,
		0,
	},
};

typedef struct {
	multiboot_hdr_t* mb;
	struct {
		u64 total;
		u64 free;
		u64 used;
		u64 reserved;
		void* heap_base;
		u64 heap_size;
	} mem_stats;
	struct {
		void* kernel_addr;
	} kernel_stats;
} __attribute__((packed)) boot_info_t;

typedef u8 (*kentry)(boot_info_t* mbi);

void c_main(multiboot_hdr_t* _mbi) {
	mbi = _mbi;
	multiboot_tag_t* tag = (multiboot_tag_t*)((u8*)mbi+8);

	for (; tag->type != MULTIBOOT_TAG_TYPE_END;
	tag = (multiboot_tag_t*) ((u8*) tag + ((tag->size + 7) & ~7))) {
		mb_hdr_length += tag->size;

		switch (tag->type) {
			case MULTIBOOT_TAG_TYPE_EFI_MMAP: {
				interpret_mmap((multiboot_tag_efi_mmap_t*)tag);
				break;
			}
			case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
				fb_tag = (multiboot_tag_framebuffer_t*) tag;
				fb_base = (void*)fb_tag->common.framebuffer_addr;
				break;
			}
		}
	}

	__asm__ volatile("cli");

	__asm__("movq $0xC0000011, %rax");
	__asm__("mov %rax, %cr0");

	__asm__("movq $0x00000768, %rax");
	__asm__("movq %rax, %cr4");

	setup_paging();

	// Kernel is to be loaded at 0xFFFFFF8000000000
	Elf64_Ehdr* ehdr = (Elf64_Ehdr*)&_binary_out_kernel_start;
	if (memcmp(ehdr->e_ident, "\127ELF", 3)) {
		printk("Kernel found!\n\r");
	} else {
		printk("No kernel found!\n\r");
	}

	Elf64_Phdr* phdrs = (Elf64_Phdr*)((u8*)ehdr + ehdr->e_phoff);

	for (
		Elf64_Phdr* phdr = phdrs;
		(u8*)phdr < (u8*)phdrs + ehdr->e_phnum*ehdr->e_phentsize;
		phdr = (Elf64_Phdr*)((u8*)phdr + ehdr->e_phentsize)
	) {
		if (phdr->p_type == PT_LOAD) {
			for (u16 page = 0; page*0x1000 < phdr->p_memsz; page++) {
				void* page_p = request_page();
				map_page((void*)phdr->p_vaddr+(page*0x1000), page_p, MAP_FLAGS_DEFAULTS);
			}

			memcpy((void*)phdr->p_vaddr, (void*)ehdr + phdr->p_offset, phdr->p_memsz);
			
			if (phdr->p_memsz > phdr->p_filesz) {
				// Set the remaining bytes to 0, as per the ELF specification
				u32 remaining = phdr->p_memsz - phdr->p_filesz;
				memset((void*)(phdr->p_vaddr + phdr->p_memsz), 0, remaining);
			}
		}
	}

	boot_info_t* info = request_page();
	info->mb = mbi;
	info->mem_stats.free = free_mem;
	info->mem_stats.used = used_mem;
	info->mem_stats.total = total;
	info->mem_stats.reserved = reserved_mem;
	info->mem_stats.heap_base = heap_base;
	info->mem_stats.heap_size = heap_size;

	printk("Kernel loaded...\n\r");
	printk("Exiting to kernel...\n\r");
	printk("Kernel returned: %d\n\r", ((kentry)ehdr->e_entry)(info));

	__asm__("cli\n hlt");
}

void __stack_chk_fail() {}
