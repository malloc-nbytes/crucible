#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

printf 'let x: i32 = 0;\nlet y: i32 = 1 + x;\n' > "$tmpdir/input.cr"

if ! (cd "$tmpdir" && "$root/src/build/debug/crucible") > "$tmpdir/stdout" 2> "$tmpdir/stderr"; then
        printf '%s\n' 'expected top-level expressions to resolve successfully' >&2
        printf 'stderr: %s\n' "$(<"$tmpdir/stderr")" >&2
        exit 1
fi

if [ -s "$tmpdir/stderr" ]; then
        printf 'unexpected stderr: %s\n' "$(<"$tmpdir/stderr")" >&2
        exit 1
fi
