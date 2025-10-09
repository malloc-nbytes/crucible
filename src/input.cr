module main where

import std.io;
import std.binds.c.stdio;
import std.binds.c.stdlib;

export proc _start(void): !
{
        let x: i32 = -1;

        cstdio::printf("%d\n", x);

        exit;
}
