module main where

import std.binds.c.stdio;

proc sum(a: i32, b: i32): i32 { return a+b; }

export proc _start(void): !
{
        let f: proc(i32, i32): i32 = sum;

        cstdio::printf("%d\n", f(1, 2));

        exit;
}
