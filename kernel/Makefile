rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

CSRC := $(call rwildcard, src, *.c)
SSRC := $(call rwildcard, src, *.s)

COBJ := $(patsubst src/%.c, out/obj/%.c.o, $(CSRC))
SOBJ := $(patsubst src/%.s, out/obj/%.s.o, $(SSRC))

CFLAGS += -ffreestanding -fshort-wchar -g -fstack-protector
LDFLAGS += -T script.ld -static -Bsymbolic -nostdlib

all: $(COBJ) $(SOBJ) kernel

clean:
	rm -rf out/*

objdir: clean
	rsync -a -f"+ */" -f"- *" src/ out/obj/
	
out/obj/%.c.o: src/%.c
	gcc $(CFLAGS) -c $^ -o $@

out/obj/%.s.o: src/%.s
	gcc $(CFLAGS) -c $^ -o $@

kernel: $(COBJ) $(SOBJ)
	ld $(LDFLAGS) $^ -o ../bootloader/kernel

.PHONY: kernel all