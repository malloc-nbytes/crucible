module main where

import std.binds.c.stdio;
import std.io;

export proc _start(void): !
{
        let u: size_t = 0;

        cstdio::printf("%zu\n", u);

        exit;
}
