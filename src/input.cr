module main where

import std.binds.c.stdio;

export proc _start(void): !
{
        let a: [i32; 5] = {0,1,2,3,4};

        -- cstdio::printf("%d\n", a[0]);

        exit;
}
