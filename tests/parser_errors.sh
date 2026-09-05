#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

printf 'extern proc broken(): i32;\n' > "$tmpdir/input.cr"

if (cd "$tmpdir" && "$root/src/build/debug/crucible") > "$tmpdir/stdout" 2> "$tmpdir/stderr"; then
        printf '%s\n' 'expected malformed procedure to fail parsing' >&2
        exit 1
fi

expected="input.cr:1:19: procedures with no parameters need to be \`void'"
actual=$(<"$tmpdir/stderr")

if [ "$actual" != "$expected" ]; then
        printf 'expected: %s\nactual:   %s\n' "$expected" "$actual" >&2
        exit 1
fi
