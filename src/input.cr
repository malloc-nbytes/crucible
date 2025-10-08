module main where

import std.binds.c.stdio;

export proc _start(void): !
{
        let x: [i32] = {1,2,3,4,5};

        let p: i32* = &x[0];

        cstdio::printf("%d\n", *p);
        p += 1;
        cstdio::printf("%d\n", *p);

        exit;
}
