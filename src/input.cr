module main where

import std.binds.c.stdio;

export proc _start(void): !
{
        let ar: [i32] = {1,2,3,4,5};
        let p: i32* = &ar[0];

        cstdio::printf("%d\n", p[0]);

        exit;
}
