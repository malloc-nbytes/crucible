#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

printf ';\n' > "$tmpdir/input.cr"

if (cd "$tmpdir" && "$root/src/build/debug/crucible") > "$tmpdir/stdout" 2> "$tmpdir/stderr"; then
        printf '%s\n' 'expected invalid expression to fail parsing' >&2
        exit 1
fi

expected="input.cr:1:1: expected expression but got \`;'"
actual=$(<"$tmpdir/stderr")

if [ "$actual" != "$expected" ]; then
        printf 'expected: %s\nactual:   %s\n' "$expected" "$actual" >&2
        exit 1
fi
