module main where

-- import std.binds.c.stdio;

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
        let p: Point = Point {
                .x = 1,
                .y = 2,
                .s = (S) {
                        .a = "foo",
                        .b = "bar",
                },
        };

        -- let copy: Point = p;

        exit;
}
