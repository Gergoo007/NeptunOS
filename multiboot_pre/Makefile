.PHONY: all clean link build pack uefi run

_CFLAGS := -c -nostdinc -nostdlib -ffreestanding -Isrc -std=c17 $(CFLAGS)
_LDFLAGS := -n -T src/link.ld $(LDFLAGS)

ASM_SRC := $(shell find . -name '*.S' -type f)
ASM_OBJ = $(patsubst ./src/%.S, out/%.s.o, $(ASM_SRC))

C_SRC := $(shell find . -name '*.c' -type f)
C_OBJ = $(patsubst ./src/%.c, out/%.c.o, $(C_SRC))

all: build pack run

clean:
	@cp out/payload.o .
	@rm -rf out/*
	@mv payload.o out/payload.o
	@rsync -a --include='*/' --exclude='*' src/ out/

out/%.c.o: src/%.c
	gcc $(_CFLAGS) -xc $^ -o $@

out/%.s.o: src/%.S
	nasm -felf64 $^ -o $@

link: clean $(C_OBJ) $(ASM_OBJ)
	ld $(_LDFLAGS) $(ASM_OBJ) $(C_OBJ) out/payload.o -o out/carrier.elf
	cp out/carrier.elf ../iso_content/kernel.elf

build: link
	./verify.sh out/carrier.elf

