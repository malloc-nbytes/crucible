module main where

import std.binds.c.stdio;

proc sum(a: i32, b: i32): i32
{
        cstdio::printf("in sum()\n");
        return a+b;
}

proc fold(f: proc(i32, i32): i32): void
{
        f(1, 2);
}

export proc _start(void): !
{
        let ar: [i32] = [1,2,3];

        exit;
}
