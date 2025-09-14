#!/bin/bash

set -xe

# TODO: add debug build option

cc="cc"
name="cruc"
files=$(find . -type f -name '*.c')
flags="-Iinclude/ -Wextra -Wall"
libs="$(forge lib)"

$cc -o $name $files $flags -O2 $libs
