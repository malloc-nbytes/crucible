module main where

import std.binds.c.stdlib;

struct S {
        a: u8*,
        b: u8*,
}

struct Point {
        x: i32,
        y: i32,
        s: S,
}

export proc _start(void): !
{
        let p: Point = (Point) {
                .x = 1,
                .y = 2,
                .s = (S) {
                        .a = null,
                        .b = null,
                },
        };

        let copy: Point = p;

        exit;
}
