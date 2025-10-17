#!/bin/bash

set -xe

FILES=$(find . -type f -name '*.c')
FLAGS="-ggdb -O0 -Iinclude/ -lforge"
cc -o cruc-debug-build $FLAGS $FILES

gdb --args ./cruc-debug-build input.cr
