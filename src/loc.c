#include "loc.h"

#include <stdio.h>
#include <string.h>

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

char *
loc_err(loc loc)
{
        char buf[512] = {0};
        snprintf(buf, sizeof(buf), "%s:%zu:%zu: error: ", loc.fp, loc.r, loc.c);
        return strdup(buf);
}
