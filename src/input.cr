module main where

import std.binds.c.stdio;

export proc _start(void): !
{

    let arr: [i32] = {1, 2, 3};

    arr[1] += 99;

    cstdio::printf("%d\n", arr[1]);

    exit;
}
