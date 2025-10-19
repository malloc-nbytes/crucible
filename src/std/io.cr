module io where

-- This module provides an interface for
-- doing all I/O operations in crucible.

import std.bytes;

export proc echo(msg: u8*): void
{
        let sz: size_t = bytes::length(msg);

        embed {
                "mov rax, 1",
                "mov rdi, 1",
                "mov rsi, {msg}",
                "mov edx, {sz}",
                "syscall",
        }
}

export proc echoln(msg: u8*): void
{
        echo(msg);
        echo("\n");
}
