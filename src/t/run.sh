#!/bin/bash

set -e

printf "\033[33m===== Running Pre-Bootstrapped Tests =====\033[0m\n"
sleep 1
pushd old
earl runner.rl -- asm
popd

printf "\033[33m===== Running Bootstrapped Tests =====\033[0m\n"
sleep 1
pushd bootstrap
/bin/bash ./run.sh
printf "\033[33m===== Done =====\033[0m\n"
