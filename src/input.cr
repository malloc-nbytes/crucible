module main where

import std.binds.c.stdlib;
import std.binds.c.stdio;

export proc _start(void): !
{
        let a: u8 = 0;
        let x: i32 = cast<i32>(a) + 1;

        exit;
}
