#include "mem.h"
#include "err.h"

#include <stdlib.h>

uint8_t *
alloc(size_t sz)
{
        if (!sz)
                return NULL;

        uint8_t *p = NULL;

        if (!(p = (uint8_t *)malloc(sz)))
                fatal("could not alloc %zu bytes", sz);

        return p;
}
