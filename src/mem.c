#include "mem.h"

#include <forge/err.h>

void *
alloc(size_t bytes)
{
        void *p = malloc(bytes);
        if (!p) {
                forge_err_wargs("could not allocate %zu bytes", bytes);
        }
        return p;
}
