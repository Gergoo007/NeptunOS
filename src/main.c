#include <multiboot2.h>

#define pixel(x, y, color) *((uint32_t*) (((uint64_t) fb_base) + 4 * bootboot.fb_width * y + 4 * x)) = color;

tag_align __attribute__((section(".multiboot"))) multiboot_hdr_t header = {
	// MB2 magic header
	0xE85250D6,
	0,
	sizeof(multiboot_hdr_t),
	(uint32_t)0 - 0xE85250D6 - 0 - sizeof(multiboot_hdr_t),

	// Tags
	// FB info tag [not supported]
	// { .type = 6 },

	// EFI System Table (amd64)
	//{ .type = 11, .size = 16 },

	// Command line
	//{ .type = 1 },

	// .info_request = {
	// 	.type = 1,
	// 	.flags = 0,
	// 	.size = sizeof(info_request),
	// },

	// End tag
	//.end_tag = { 0, 0, 8 },
};

uint16_t x = 0, y = 0;

void putc(unsigned char c, int x, int y) {
	uint16_t attrib = (0x0 << 4) | (0xf & 0x0F);
	volatile uint16_t * where;
	where = (volatile uint16_t*)0xB8000 + (y * 80 + x) ;
	*where = c | (attrib << 8);
}

void puts(char* str) {
	do {
		if (*str == '\n') {
			y++;
			x = 0;
			continue;
		}
		putc(*str, x, y);
		if (x > 80) {
			x = 0;
			y++;
		} else
			x++;
	} while (*(str++) != '\0');
}

char* uint_to_str(uint64_t value, char* result, int base) {
		// check that the base if valid
		if (base < 2 || base > 36) { *result = '\0'; return result; }

		char* ptr = result, *ptr1 = result, tmp_char;
		uint64_t tmp_value;

		do {
				tmp_value = value;
				value /= base;
				*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
		} while ( value );

		// Apply negative sign
		if (tmp_value < 0) *ptr++ = '-';
		*ptr-- = '\0';
		while(ptr1 < ptr) {
				tmp_char = *ptr;
				*ptr--= *ptr1;
				*ptr1++ = tmp_char;
		}
		return result;
}

void c_main(uint32_t mbi) {
	// Print OK using VGA
	*(uint32_t*)0xb8000 = 0x2f4b2f4f;

	puts("Hello, world!\n");

	multiboot_tag_t* tag = (multiboot_tag_t*)(uint64_t)(mbi+8);
	
	for (; tag->type != MULTIBOOT_TAG_TYPE_END;
	tag = (multiboot_tag_t*) ((uint8_t*) tag + ((tag->size + 7) & ~7))) {
		switch (tag->type) {
			case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
				puts("Found it.\n");
				multiboot_tag_framebuffer_t* fbinfo = (multiboot_tag_framebuffer_t*)tag;
				void* addr = (void*)fbinfo->common.framebuffer_addr;
				char buffer[256];
				uint_to_str(fbinfo->common.framebuffer_bpp, buffer, 10);
				puts("BPP: ");
				puts(buffer);
				uint_to_str(fbinfo->common.framebuffer_type, buffer, 10);
				puts("Type: ");
				puts(buffer);
				break;
			}
		}
	}

	while(1);
}
