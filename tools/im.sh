objdump --disassemble kernel/modules/$1/out/$1.ko --disassembler-color=on --visualize-jumps=color -S --demangle=auto | less -r

