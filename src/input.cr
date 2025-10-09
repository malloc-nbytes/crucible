module main where

import std.binds.c.stdio;
import std.io;

export proc _start(void): !
{
        let a: i64 = 0;
        let b: i32 = (i32)a;

        exit;
}
