#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

printf 'let value: i32 = missing;\n' > "$tmpdir/input.cr"

if (cd "$tmpdir" && "$root/src/build/debug/crucible") > "$tmpdir/stdout" 2> "$tmpdir/stderr"; then
        printf '%s\n' 'expected unresolved identifier to fail resolution' >&2
        exit 1
fi

expected="input.cr:1:18: identifier \`missing' has not been defined"
actual=$(<"$tmpdir/stderr")

if [ "$actual" != "$expected" ]; then
        printf 'expected: %s\nactual:   %s\n' "$expected" "$actual" >&2
        exit 1
fi
