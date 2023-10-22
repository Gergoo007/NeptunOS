#include <lib/kinfo.h>
#include <lib/sprintf.h>
#include <init/multiboot.h>
#include <serial/serial.h>

u8 kmain(kernel_info_t* info) {
	char tmp[256];
	sprintf(tmp, "qwertzui%s\n\rA szám: %d\n\rTMP címe: %p\n\r", "GHJF", 234);
	puts(tmp);

	sprintk("Hello world sprintk-ból! %d %x %p\n\r", 123, 0xabc, &tmp);

	// Multiboot tag-ek feldolgozása
	mb_parse_tags(info->mb_hdr_addr);

	// Zöld téglalap

	return 88;
}
