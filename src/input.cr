module main where

import std.binds.c.stdio;

macro F(a)
        if (a == 0) { printf("a is 0\n"); }
        else        { printf("a is something else\n"); }
end

export proc _start(void): !
{
        F(1);

        exit;
}
