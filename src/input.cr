module main where

import std.binds.c.stdio;

export proc _start(void): !
{
        let i: size_t = 0;
        for (i = 0; i < 10; i += 1);
        cstdio::printf("%zu\n", i);

        exit;
}
