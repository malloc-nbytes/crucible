module main where

import std.binds.c.stdio;

export proc _start(void): !
{

    let arr: [i32] = {1, 2, 3};

    cstdio::printf("%d\n", arr[1]);

    exit;
}
