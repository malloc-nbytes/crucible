module main where

import std.binds.c.stdio;

export proc _start(void): !
{
    let arr: [i32] = {1, 2, 3};

    let x: i32 = arr[0];

    exit;
}
