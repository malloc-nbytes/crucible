module bytes where

-- This module covers different byte procedures 'aka cstrings' (u8*).

export proc length(s: u8*): size_t
{
        let i: size_t = 0;
        for (i = 0; s[i]; i += 1);
        return i;
}

export proc copy(dst: u8*, src: u8*): void
{
        for (let i: size_t = 0; src[i]; i += 1) {
                dst[i] = src[i];
        }
}

export proc cat(dst: u8*, src: u8*): void
{
        while (*dst) dst += 1;
        for (let i: size_t = 0; src[i]; i += 1) {
                dst[i] = src[i];
        }
}

export proc eq(s0: u8*, s1: u8*): bool
{
        while (*s0 && *s1) {
                if (*s0 != *s1) return false;
                s0 += 1;
                s1 += 1;
        }

        return !*s0 && !*s1;
}

export proc isalpha(c: u8): bool
{
        return c >= 64 && c <= 122;
}

export proc isdigit(c: u8): bool
{
        return c >= 48 && c <= 57;
}

export proc isalnum(c: u8): bool
{
        isdigit(c) || isalpha(c);
}

-- export proc atoi(s: u8*): i32
-- {
--         let result: i32 = 0;
--         let sign: i32 = 1;

--         while (*s == ' ') s += 1;

--         if (*s == '-') {
--                 sign = -1;
--                 s += 1;
--         } else if (*s == '+') {
--                 s += 1;
--         }

--         while (isdigit(*s)) {
--                 result = (u8)result * 10 + (*s - '0');
--                 s += 1;
--         }

--         return sign * result;
-- }

