module dev where

-- This module provides a development interface.

import std.io;

export proc TODO(msg: u8*): !
{
    io::echo("[crucible] TODO: ");
    io::echoln(msg);
    exit 1;
}
