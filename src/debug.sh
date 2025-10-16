#!/bin/bash

set -xe

CFILES=$(find . -type f -name '*.c')
CFLAGS="-ggdb -O0 -Iinclude/ -lforge"
cc -o cruc-debug-build $CFILES $CFLAGS
gdb --args ./cruc-debug-build input.cr
