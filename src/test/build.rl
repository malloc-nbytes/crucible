#!/usr/bin/env earl

module Build

$"nasm -f elf64 -g -F dwarf out.s -o out.o";
$"ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 -lc -o out out.o";
# $"gcc -g -O0 -no-pie out.o -o out";
