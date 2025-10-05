module other;

import std.c;

export proc sum(a: i32, b: i32): i32 {
    c::printf("in other::sum()\n");
    return a+b;
}
