#include "ds/strv.h"

#include <assert.h>
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
                return s0.len > s1.len ? s0.len-s1.len : s1.len - s0.len;
        return strncmp(s0.st, s1.st, s0.len);
}

int
strv_cmp2(strv        s0,
          const char *s1)
{
        size_t s1n = strlen(s1);
        if (s1n != s0.len)
                return s1n > s0.len ? s1n-s0.len : s0.len-s1n;
        return strncmp(s0.st, s1, s0.len);
}

const char *
strv_scstr(strv s)
{
        constexpr size_t MAX_LEN      = 256;
        static char      buf[MAX_LEN] = {0};

        assert(s.len < MAX_LEN);

        (void)memcpy(buf, s.st, s.len);

        buf[s.len] = 0;

        return buf;
}
