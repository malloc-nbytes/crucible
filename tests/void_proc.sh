#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

printf 'proc f(void): void\n{\n\n}\n' > "$tmpdir/input.cr"

if ! (cd "$tmpdir" && "$root/src/build/debug/crucible") > "$tmpdir/stdout" 2> "$tmpdir/stderr"; then
        printf 'expected void-marked zero-parameter procedure to parse successfully\n' >&2
        printf 'stderr: %s\n' "$(<"$tmpdir/stderr")" >&2
        exit 1
fi

if [ -s "$tmpdir/stderr" ]; then
        printf 'unexpected stderr: %s\n' "$(<"$tmpdir/stderr")" >&2
        exit 1
fi
