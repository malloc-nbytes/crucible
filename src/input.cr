module main where

import std.binds.c.stdio;

macro F
        if (x == 0) {
                x = 9;
        } else {
                x = 100;
        }
end

macro C
F
end

export proc _start(void): !
{
        let x: i32 = 1;
        C;

        cstdio::printf("x: %d\n", x);

        exit;
}
