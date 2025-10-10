module main where

import std.io;

export proc _start(void): !
{
        io::echof("foo %s bar\n", "test");

        exit;
}
