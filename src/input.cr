module main where

import std.binds.c.stdio;

export proc _start(void): !
{
        let print: proc(u8*): i32 = cstdio::printf;

        print("hello\n");

        exit;
}
