module main where

import std.io;
import std.binds.c.stdio;
import std.binds.c.stdlib;

export proc _start(void): !
{
        let s: u8* = "hello";

        let c1: u8 = *s;
        let c2: u8 = s[1];

        cstdio::printf("%d\n", c1 == c2);

        exit;
}
