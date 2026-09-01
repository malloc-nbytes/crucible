#!/bin/bash

set -e

pushd stage1
echo '*** Stage 1 ***'
make -B
popd

pushd stage2
echo '*** Stage 2 ***'
./build.sh
popd
