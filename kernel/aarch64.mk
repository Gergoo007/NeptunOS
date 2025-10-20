ifeq ($(MANAGE_ARCH_STUFF),1)
SSRCS += $(subst src/,,$(shell find src/arch/aarch64 -name *.s))
SOBJS += $(patsubst %.s,out/%.s.o,$(SSRCS))

CCSRCS += $(subst src/,,$(shell find src/arch/aarch64 -name *.cc))
endif

CXXFLAGS += -mgeneral-regs-only -ffreestanding -nostdlib -nostdinc \
	-Isrc -xc++ -std=gnu++23 -Wall -Wextra -Wshadow -Wno-address-of-packed-member \
	-Wno-unused-parameter -fno-stack-protector -O3 -g -Wno-multichar \
	-Wno-packed-bitfield-compat -fno-omit-frame-pointer -fno-rtti -fno-exceptions
ASFLAGS +=
LDFLAGS += -z max-page-size=0x1000 --no-gc-sections

CC := aarch64-none-elf-g++
LD ?= aarch64-none-elf-ld
AS ?= aarch64-none-elf-gcc
STRIP ?= aarch64-none-elf-strip
