module main where

import std.binds.*;

export proc _start(void): !
{
        cstdio::printf("foo\n");
        cstdlib::malloc(1000);

        exit;
}
