SSRCS += $(subst src/,,$(shell find src/arch/amd64 -name *.s)) # $(subst src/,,$(shell find src -name *.s -printf '%P\n'))
SOBJS += $(patsubst %.s,out/%.s.o,$(SSRCS))

CCSRCS += $(subst src/,,$(shell find src/arch/amd64 -name *.cc))

# no-red-zone: painful addition
CXXFLAGS += -mgeneral-regs-only -ffreestanding -nostdlib -nostdinc \
	-Isrc -xc++ -std=gnu++23 -Wall -Wextra -Wshadow -Wno-address-of-packed-member \
	-Wno-unused-parameter -fno-stack-protector -mno-red-zone -O0 -g -Wno-multichar \
	-Wno-packed-bitfield-compat -msse2 -fno-omit-frame-pointer -fno-rtti -fno-exceptions \
	-mcmodel=large -mabi=sysv
ASFLAGS +=
LDFLAGS += -T linker.amd64.ld --no-gc-sections -static

CC := x86_64-elf-g++
LD ?= x86_64-elf-ld
AS ?= x86_64-elf-gcc

# CC := clang++
# LD ?= ld.lld
# AS ?= clang
