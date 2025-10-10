module string where

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
