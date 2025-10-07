module assert where

import std.binds.c.string;
import std.binds.c.stdio;

export proc is_true(b: i32): void
{
        if (b == 0) {
                cstdio::printf("assert_true(): assertion failure");
        }
}

export proc is_false(b: i32): void
{
        if (b != 0) {
                cstdio::printf("assert_false(): assertion failure");
        }
}

export proc i32eq(i0: i32, i1: i32): void
{
        if (i0 != i1) {
                cstdio::printf("assert_i32eq(): assertion failure [left=%d, right=%d]\n", i0, i1);
        }
}

export proc u8ptreq(s0: u8*, s1: u8*): void
{
        if (cstring::strcmp(s0, s1)) {
                cstdio::printf("assert_u8ptreq(): assertion failure [left=%s, right=%s]\n", s0, s1);
        }
}
