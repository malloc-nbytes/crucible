#!/usr/bin/env earl

### Note: This script is built with the EARL language [https://github.com/malloc-nbytes/earl].
###       You will need this to use this script, or just use the associated build.sh script for BASH.

module Build

set_flag("-xe");

let debug, test = (false, false);
try { debug = ("debug", "d", "g", "ggdb").contains(argv()[1]); }
try { test = ("test", "t").contains(argv()[1]); }

let cc, name, files, flags, libs = (
    "cc",
    "cruc",
    "*.c",
    "-Iinclude/ -Wextra -Wall",
    "-lforge",
);

if debug {
    $f"{cc} -o {name} {files} {flags} -ggdb -O0 {libs}";
    $f"gdb ./{name}";
} else if test {
    cd("t");
    $"/bin/bash run.sh";
} else {
    $f"{cc} -o {name} {files} {flags} -O2 {libs}";
}
