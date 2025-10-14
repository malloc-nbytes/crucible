module main where

import std.binds.c.stdlib;
import std.binds.c.stdio;

proc f(p: i32*, n: size_t): i32
{
        let p2: i32* = p-1;
        return *p2;
}

export proc _start(void): !
{
        exit;
}
