module main where

import std.binds.c.stdio;

export proc _start(void): !
{
        let x: i32 = 0;

        cstdio::printf("HERE: %d\n", !x);

        exit;
}
