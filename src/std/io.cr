module io where

-- This module provides an interface for
-- doing all I/O operations in crucible.

-- TODO: Remove C dependency.
import std.binds.c.stdlib;

import std.string;

export proc echo(msg: u8*): void
{
        let sz: size_t = string::len(msg);

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

export proc echof(fmt: u8*, s: u8*): void
{
        let buf: u8*    = (u8*)cstdlib::malloc(1);
        let len: size_t = 0;
        let cap: size_t = 1;

        for (let i: size_t = 0; fmt[i]; i += 1) {
                if (fmt[i] == '%' && fmt[i+1] && fmt[i+1] == 's') {
                        while (*s) {
                                if (len >= cap) {
                                        cap = cap*2;
                                        buf = (u8*)cstdlib::realloc((void*)buf, cap);
                                }
                                buf[len] = *s;
                                len += 1;
                                s += 1;
                        }
                        i += 1;
                } else {
                        if (len >= cap) {
                                cap = cap*2;
                                buf = (u8*)cstdlib::realloc((void*)buf, cap);
                        }
                        buf[len] = fmt[i];
                        len += 1;
                }
        }

        echo((u8*)buf);

        cstdlib::free((void*)buf);
}
