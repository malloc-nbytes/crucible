module main where

-- import std.binds.c.stdio;

struct S {
        a: u8*,
        b: u8*,
}

struct Point {
        x: i32,
        s: S,
        y: i32,
}

export proc _start(void): !
{
        -- let s: S = S {
        --         .a = "hello",
        --         .b = "world",
        -- };

        let p: Point = Point {
                .x = 1,
                .s = S {
                        .a = "foo",
                        .b = "bar",
                },
                .y = 2,
        };

        exit;
}
