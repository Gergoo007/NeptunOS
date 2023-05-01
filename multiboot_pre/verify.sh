#!/bin/bash

if grub-file --is-x86-multiboot2 $1; then
	echo "Passed"
else
	echo "Invalid"
fi
