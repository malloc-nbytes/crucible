module main where

import std.binds.c.stdio;
import std.math;

export proc _start(void): !
{
    let x: i32 = 1;
    cstdio::printf("%d\n", x);

    exit;
}
