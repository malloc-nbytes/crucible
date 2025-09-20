module Runner

import "std/system.rl"; as sys

let files = sys::ls(".").filter(!= "./runner.rl");

foreach f in files {
    $f"../cruc ./{f}";
    $f"nasm -f elf64 -g -F dwarf {f}.s -o {f}.o";
    $f"ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 -lc -o {f} {f}.o";
}
