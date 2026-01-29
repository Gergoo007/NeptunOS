ifeq ($(MANAGE_ARCH_STUFF),1)
SSRCS += $(subst src/,,$(shell find src/arch/amd64 -name *.s)) # $(subst src/,,$(shell find src -name *.s -printf '%P\n'))
SOBJS += $(patsubst %.s,out/%.s.o,$(SSRCS))

CCSRCS += $(subst src/,,$(shell find src/arch/amd64 -name *.cc))
endif

# no-red-zone: painful addition
CXXFLAGS += -mgeneral-regs-only -ffreestanding -nostdlib -nostdinc \
	-Isrc -xc++ -std=gnu++2c -Wall -Wextra -Wshadow -Wno-address-of-packed-member \
	-Wno-unused-parameter -fno-stack-protector -mno-red-zone -O0 -g -Wno-multichar \
	-Wno-packed-bitfield-compat -msse2 -fno-omit-frame-pointer -fno-rtti -fno-exceptions \
	-MMD -MP -DDEBUG -fno-lto -fstack-protector-strong -Wno-format -Wno-implicit-fallthrough
ASFLAGS += -fno-lto
LDFLAGS += --no-gc-sections

CC := x86_64-elf-g++
LD ?= x86_64-elf-ld
AS ?= x86_64-elf-gcc
STRIP ?= x86_64-elf-strip

# CC := clang++ -fno-lto
# LD ?= ld.lld -m elf_x86_64
# AS ?= clang -fno-integrated-as -fno-lto
# STRIP ?= x86_64-elf-strip
