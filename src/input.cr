module main where

import std.binds.c.stdio;

export proc _start(void): !
{
        cstdio::printf("sum: %d\n", math::sum(1, 2));

        exit;
}
