module main where

import std.binds.c.stdio;

export proc _start(void): !
{
    if (0 || 1) {
        cstdio::printf("foo\n");
    }

    exit;
}
