#!/usr/bin/env earl

module Build

set_flag("-xe");

let debug = false;
try { debug = ("debug", "d", "g", "ggdb").contains(argv()[1]); }

let cc, name, files, flags, libs = (
    "cc",
    "cruc",
    "*.c",
    "-Iinclude/ -Wextra -Wall",
    "$(forge lib)",
);

if debug {
    $f"{cc} -o {name} {files} {flags} -ggdb -O0 {libs}";
} else {
    $f"{cc} -o {name} {files} {flags} -O2 {libs}";
}
