module main where

import std.binds.c.stdio;

macro F(a, b) {
        cstdio::printf("macro: %d\n", a + b);
        x = a+b;
} end

macro SUM(x, y) x+y end

export proc _start(void): !
{
        let x: i32 = 1;

        F(1, 3+4);

        cstdio::printf("done: %d\n", x);

        exit;
}
