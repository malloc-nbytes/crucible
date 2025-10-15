module main where

import std.binds.c.stdio;

macro F(a, b)
cstdio::printf("here: %d\n", a+b);
end

export proc _start(void): !
{
        let x: i32 = 1;

        F(x, 1);

        exit;
}
