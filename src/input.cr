module main where

import std.binds.c.stdio;

proc sum(a: i32, b: i32): i32
{
        cstdio::printf("in sum()\n");
        return a+b;
}

export proc _start(void): !
{
        let ar: [proc(i32, i32): i32] = {sum, sum, sum};

        for (let i: size_t = 0; i < 3; i += 1) {
                cstdio::printf("%d\n", ar[i]((i32)i, (i32)i+1));
        }

        exit;
}
