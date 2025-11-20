#include <util/stacktrace.hh>
#include <util/ksyms.hh>
#include <util/string.hh>
#include <util/mem.hh>
#include <types.hh>

extern "C" void kmain();

void demangle(const char* input, char* demangled) {
	// u32 asd = 0;
	// while (input[asd]) {
	// 	demangled[asd] = input[asd];
	// 	asd++;
	// }
	// demangled[asd++] = ' ';
	// demangled[asd++] = '-';
	// demangled[asd++] = '>';
	// demangled[asd++] = ' ';

	// demangled += asd;

	// if (!strncmp("_Z", input, 2)) {
	// 	input += 2;
	// 	while (*input) {
	// 		bool nested = false;

	// 		// nested name, például turi::ip::ip
	// 		if (*input == 'N') {
	// 			nested = true;
	// 			input++;
	// 		}

	// 		u64 chars;
	// 		u32 len = str_to_uint(input, &chars);
	// 		input += chars;

	// 		memcpy((void*)demangled, (void*)input, len);
	// 		input += len;
	// 		demangled += len;

	// 		if (nested) {
	// 			*(demangled++) = ':';
	// 			*(demangled++) = ':';
	// 		}
	// 	}

	// 	*demangled = 0;
	// } else {
	// 	strcpy(input, demangled);
	// }
}

void stacktrace() {
	stackframe_t* s;
	asm volatile ("movq %%rbp, %0" : "=r"(s));

	printk("Stacktrace:\n");
	// char demangled[128];

	while (s->rip != (u64)kmain && s->rip) {
		Elf64_Sym& closest = ksyms[0];
		for (u64 i = 1; i < num_ksyms; i++) {
			if (s->rip >= ksyms[i].st_value && s->rip <= ksyms[i].st_value + ksyms[i].st_size) {
				closest = ksyms[i];
			}
		}

		// demangle(kstrtab + closest.st_name, demangled);
		printk("[..%08X] %s +%llx\n", (u32)s->rip, kstrtab + closest.st_name, s->rip - closest.st_value);

		s = s->rbp;
	}
}
