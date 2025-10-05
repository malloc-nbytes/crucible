module main;

import std.c;
import other;

export proc _start(void): !
{
    c::printf("Hello, World\n");

    other::sum(1, 2);

    exit;
}
