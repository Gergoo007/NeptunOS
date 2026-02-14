# Disable idiotic invisible default variables like CC
MAKEFLAGS += --no-builtin-variables

PROGRAMNAME := $(notdir $(CURDIR))

build:
	@cargo build --target ../target.json -r

install: build
	../../tools/mount.sh
	cp target/target/release/$(PROGRAMNAME) /mnt/misc/bin/
	sync

.PHONY: modules
$(V).SILENT:
