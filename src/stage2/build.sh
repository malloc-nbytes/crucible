#!/bin/bash

set -e

# pushd ../stage1
# make -B
# popd

cc -c io.c -o io.o

../stage1/cruc.stage1 ./main.cr -o cruc.stage2 -obj io.o
