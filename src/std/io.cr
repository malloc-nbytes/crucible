module io where

-- This module provides an interface for
-- doing all I/O operations in crucible.

-- TODO: Remove C dependency.
import std.binds.c.string;

export proc echo(msg: u8*): void
{
    let sz: i32 = cstring::strlen(msg);

    embed {
        "mov rax, 1",
        "mov rdi, 1",
        "mov rsi, [rbp-8]",
        "mov edx, [rbp-12]",
        "syscall"
    }
}

export proc echoln(msg: u8*): void
{
    echo(msg);
    echo("\n");
}
