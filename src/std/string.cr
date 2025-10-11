module string where

export proc len(s: u8*): size_t
{
        let i: size_t = 0;
        for (i = 0; s[i]; i += 1) {}
        return i;
}

export proc eq(s0: u8*, s1: u8*): bool
{
        while (*s0 && *s1) {
                if (!*s0 && *s1)  return false;
                if (*s0 && !*s1)  return false;
                if (*s0 != *s1)   return false;
                s0 += 1;
                s1 += 1;
        }

        return !*s0 && !*s1;
}
