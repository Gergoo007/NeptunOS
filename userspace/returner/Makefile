SRCS := $(wildcard src/*.c) $(wildcard src/*.s)
CFLAGS := -ffreestanding -fPIE -static -nostdlib -nostdinc

CC ?= cc

all: compile todisk

compile:
	$(CC) $(CFLAGS) -T link.ld $(SRCS) -o returner

todisk:
	cp returner ../../disk.img
