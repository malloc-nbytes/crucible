#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

for expression in '1 + x' 'x + 1'; do
        printf 'proc f(void): void {\nlet x: i32 = 0;\nlet y: i32 = %s;\n}\n' "$expression" > "$tmpdir/input.cr"

        if ! (cd "$tmpdir" && "$root/src/build/debug/crucible") > "$tmpdir/stdout" 2> "$tmpdir/stderr"; then
                printf 'expected expression `%s` to resolve successfully\n' "$expression" >&2
                printf 'stderr: %s\n' "$(<"$tmpdir/stderr")" >&2
                exit 1
        fi

        if [ -s "$tmpdir/stderr" ]; then
                printf 'unexpected stderr for `%s`: %s\n' "$expression" "$(<"$tmpdir/stderr")" >&2
                exit 1
        fi
done
