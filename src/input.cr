module main where

import std.binds.c.stdlib;
import std.binds.c.stdio;
import std.binds.c.string;
import std.math;

export proc _start(void): !
{
    let s1: i32 = 1;
    let s2: i32 = 2;
    cstdio::printf("sum: %d\n", math::sum(s1, s2));
    cstring::strcmp("foo", "foo");

    exit;
}
