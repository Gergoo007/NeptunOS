CSRC :=	$(shell find src -name '*.c' -printf '%P\n')
SSRC :=	$(shell find src -name '*.s' -printf '%P\n')

COBJS := $(patsubst %.c,out/%.c.o,$(CSRC))
SOBJS := $(patsubst %.s,out/%.s.o,$(SSRC))

CC ?= cc
AS ?= as
LD ?= ld.gold

CFLAGS := -xc -std=gnu17 -ffreestanding -nostdinc -nostdlib -mno-red-zone -mcmodel=large -Wall -Wextra -Isrc
LDFLAGS := -static -T src/ldscript.ld

NULL :=
TAB := $(NULL)<tab>$(NULL)

all: clean kernel store

clean:
	@find out -name "*.o" -type f -delete

out/%.c.o: src/%.c
	@$(CC) $(CFLAGS) -c $^ -o $@
	@echo -e "  > $(CC) $^ $@)"

out/%.s.o: src/%.s
	@$(AS) -c $^ -o $@
	@echo -e "  > $(AS) $^ $@)"

kernel: $(COBJS) $(SOBJS)
	@$(LD) $(LDFLAGS) $^ -o out/kernel
	@echo -e "  > $(LD) $^ $@)"

store:
	@$(LD) -r -b binary -o out/payload.o out/kernel
	@echo -e "  > $(LD) $^ $@)"
