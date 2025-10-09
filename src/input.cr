module main where

import std.binds.c.stdio;
import std.io;

export proc _start(void): !
{
        -- let a: u8 = 1;
        -- let b: i32 = (i32)a;

        -- let x: i8 = 1;
        -- let y: u32 = (u32)x;

        let t1: i32 = 1;
        let t2: u8 = (u8)t1;

        exit;
}
