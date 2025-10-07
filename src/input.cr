module main where

import std.binds.c.stdio;
import std.math;

proc f1(void): i32 { return 2; }
proc f2(void): i32 { return 3; }

export proc _start(void): !
{
        let x: i32 = 0;

        exit;
}
