CSRC :=	$(shell find src -name '*.c' -printf '%P\n')
SSRC :=	$(shell find src -name '*.s' -printf '%P\n')

COBJS := $(patsubst %.c,out/%.c.o,$(CSRC))
SOBJS := $(patsubst %.s,out/%.s.o,$(SSRC))

CC ?= cc
AS ?= as
LD ?= ld.gold

CFLAGS := -xc -std=gnu17 -ffreestanding -nostdinc -nostdlib -mno-red-zone -mcmodel=large -Wall -Wextra
LDFLAGS := -static -T src/ldscript.ld

all: kernel

out/%.c.o: src/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

out/%.s.o: src/%.s
	$(AS) -c $^ -o $@

kernel: $(COBJS) $(SOBJS)
	$(LD) $(LDFLAGS) $^ -o out/kernel
