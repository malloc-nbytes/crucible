#!/bin/sh
set -eu

compiler=$(realpath "$1")
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

source="$tmpdir/main.cr"
output="$tmpdir/program"
object_a="$tmpdir/helper_a.o"
object_b="$tmpdir/helper_b.o"

printf '%s\n' 'int helper_b(void) { return 5; }' > "$tmpdir/helper_b.c"
printf '%s\n' 'int helper_b(void); int helper_a(void) { return helper_b() + 32; }' > "$tmpdir/helper_a.c"
cc -c -o "$object_b" "$tmpdir/helper_b.c"
cc -c -o "$object_a" "$tmpdir/helper_a.c"
printf '%s\n' 'extern proc helper_a(): i32; export proc main(): i32 { return helper_a(); }' > "$source"

"$compiler" "$source" -obj "$object_a" -obj "$object_b" -o "$output"
test -x "$output"
set +e
timeout 10s "$output"
status=$?
set -e
test "$status" -eq 37
