#include "loc.h"

#include <stdio.h>
#include <string.h>

loc
loc_from(size_t      r,
         size_t      c,
         const char *path)
{
        return (loc) {
                .r    = r,
                .c    = c,
                .path = path,
        };
}

const char *
loc_cstr(loc l)
{
        static char buf[256];

        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%s:%zu:%zu", l.path, l.r, l.c);
        return buf;
}
