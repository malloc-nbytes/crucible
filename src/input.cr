module main where

import std.binds.c.stdio;

export proc _start(void): !
{
        let ar: [i32] = {1, 2, 3};
        let p: i32* = &ar[0];
        let x: i32 = 1;
        let p2: i32* = p+(size_t)x;

        cstdio::printf("%d\n", *p2);

        exit;
}
