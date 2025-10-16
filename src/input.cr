module main where

import std.binds.c.stdio;

struct Point {
        x: i32,
        y: i32,
}

export proc _start(void): !
{
        let p: Point = Point {
                .x = 1,
                .y = 2,
        };

        exit;
}
