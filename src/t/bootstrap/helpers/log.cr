module log where

import std.binds.c.stdio;

export proc id(test_id: u8*): void
{
        cstdio::printf("[CRUCIBLE T] %s ... ", test_id);
}
