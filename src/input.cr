module main where

import std.binds.c.stdio;

export proc _start(void): !
{
    let ar: [i32] = {1+2, 2+3*4, 99};

    exit;
}
