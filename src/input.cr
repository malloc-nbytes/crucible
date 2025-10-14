module main where

import std.binds.c.stdio;
import std.bytes;

export proc atoi(s: u8*): i32
{
        let result: i32 = 0;
        let sign: i32 = 1;

        while (*s == ' ') s += 1;

        if (*s == '-') {
                sign = -1;
                s += 1;
        } else if (*s == '+') {
                s += 1;
        }

        while (bytes::isdigit(*s)) {
                result = result * 10 + (i32)(*s - '0');
                s += 1;
        }

        return sign * result;
}

export proc _start(void): !
{
        exit;
}
