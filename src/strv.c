#include "ds/strv.h"

#include <string.h>

strv
strv_from(const char *st,
          size_t      len)
{
        strv s = {0};
        s.st   = st;
        s.len  = len;

        return s;
}

int
strv_cmp1(strv s0, strv s1)
{
        if (s0.len != s1.len)
                return 0;
        return strncmp(s0.st, s1.st, s0.len);
}

int
strv_cmp2(strv        s0,
          const char *s1)
{
        if (strlen(s1) != s0.len)
                return 0;
        return strncmp(s0.st, s1, s0.len);
}
