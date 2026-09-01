#!/bin/bash

set -xe

pushd ../stage1

make -B

popd

../stage1/cruc.stage1 ./main.cr -o cruc.stage2
