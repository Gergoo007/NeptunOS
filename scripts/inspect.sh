#!/bin/bash

make build
objdump --disassemble ../kernel/out/kernel | nvim