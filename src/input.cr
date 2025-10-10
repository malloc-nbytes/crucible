module main where

import std.binds.c.stdio;
import std.binds.c.string;
import std.binds.c.stdlib;

import std.io;

proc eq(s0: u8*, s1: u8*): bool
{
        while (*s0 && *s1) {
                -- cstdio::printf("here: %c %c\n", *s0, *s1);

                if (!*s0 && *s1)  return false;
                if (*s0 && !*s1)  return false;
                if (*s0 != *s1)   return false;
                s0 += 1;
                s1 += 1;
        }

        return 1;
}

export proc _start(void): !
{
        cstdio::printf("%d\n", (i32)eq("foo", "foo"));

        exit;
}
