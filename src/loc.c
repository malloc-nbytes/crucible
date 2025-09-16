#include "loc.h"

#include <stdio.h>

loc
loc_create(const char *fp,
           size_t      r,
           size_t      c)
{
        return (loc) {
                .fp = fp,
                .r = r,
                .c = c,
        };
}

const char *
loc_err(loc loc)
{
        // file:R:C: error: msg
        // TODO: buffer overflow
        static char buf[256] = {0};
        sprintf(buf, "%s:%zu:%zu: error: ", loc.fp, loc.r, loc.c);
        return buf;
}
