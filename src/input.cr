module main where

import std.io;

proc f1(void): i32
{
        io::echoln("in f1()");
        return 0;
}

proc f2(void): i32
{
        io::echoln("in f2()");
        return 1;
}

export proc _start(void): !
{
        if (f1() || f2())
                io::echoln("HERE");

        exit;
}
