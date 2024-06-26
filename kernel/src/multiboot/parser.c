#include <multiboot/parser.h>

void mb_parse_tags(u32 hdrp) {
	mb_tag_base* tag = (mb_tag_base*)((u64)hdrp+8);

	mb_tag_fb* fb_tag = NULL;
	mb_tag_memmap* mmap_tag = NULL;
	mb_tag_rsdp* rsdp_tag = NULL;

	while (tag->type) {
		switch (tag->type) {
			case MB_TAG_FB: {
				fb_tag = (mb_tag_fb*)tag;
				break;
			}
			case MB_TAG_MEMMAP: {
				mmap_tag = (mb_tag_memmap*)tag;
				break;
			}
			case MB_TAG_KERNEL_ADDR: {
				kinfo->preload_addr = ((mb_tag_kaddr*)tag)->addr;
				break;
			}
			case MB_TAG_XSDP: {
				rsdp_tag = (mb_tag_rsdp*)tag;
				break;
			}
			case MB_TAG_rsdp: {
				rsdp_tag = (mb_tag_rsdp*)tag;
				break;
			}
		}

		tag = (mb_tag_base*)((u64)tag + ((tag->size + 7) & ~7));
	}

	if (fb_tag)
		fb_init(fb_tag->fb_addr, fb_tag->fb_width, fb_tag->fb_height, fb_tag->fb_bpp);

	if (mmap_tag)
		pmm_init(mmap_tag);

	for (u64 page = 0; page < (fb_tag->fb_width*fb_tag->fb_height*fb_tag->fb_bpp/1024); page++)
		pmm_set_used(fb_tag->fb_addr+page*PAGESIZE, 1);

	vmm_init();

	con_init(0xffffffff, 0xff000000);

	gdt_init();
	idt_init();
	tss_init();

	if (rsdp_tag)
		acpi_init(rsdp_tag);
}
