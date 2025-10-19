module dev where

-- This module provides a development interface.

import std.io;

export proc todo(msg: u8*): !
{
    io::echof("[crucible] TODO: %s\n", msg);
    exit 1;
}
