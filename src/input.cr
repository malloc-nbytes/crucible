module main where

import std.binds.c.stdio;
import std.io;

export proc _start(void): !
{
        let x: u8 = 97;
        cstdio::printf("%d\n", x);

        exit;
}
