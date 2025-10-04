#!/bin/bash

set -e

function info() {
    msg="$1"
    printf "\033[33m===== ${msg} =====\033[0m\n"
}

function cleanup() {
    info "Removing Artifacts"

    set -x
    local -a asm_files=($(find . -type f -name '*.asm'))
    if [[ ${#asm_files[@]} -gt 0 ]]; then
        rm -f "${asm_files[@]}"
    fi

    local -a o_files=($(find . -type f -name '*.o'))
    if [[ ${#o_files[@]} -gt 0 ]]; then
        rm -f "${o_files[@]}"
    fi

    local -a bin_files=($(find . -type f -name '*.bin'))
    if [[ ${#bin_files[@]} -gt 0 ]]; then
        rm -f "${bin_files[@]}"
    fi
    set +x
}

function compile() {
    info "Building Compiler"
    sleep 1
    pushd ../../
    ./build.rl
    popd
}

function run_tests() {
    info "Compiling Test Suite"
    set -x; ../../cruc ./main.cr -o TEST.bin; set +x
    info "Running tests"
    ./TEST.bin
}

cleanup
compile
run_tests
