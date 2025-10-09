module main where

import std.binds.c.stdio;

export proc _start(void): !
{
        let s: [u8] = {'a', 'b', 'c', 0};
        s[0] = 'c';

        cstdio::printf("%s\n", s);

        exit;
}
