#include "kwds.h"

#include <stddef.h>
#include <string.h>

const char *g_kwds[]  = KWDS;
const char *g_types[] = KWD_TYPES;

int
kwds_iskw(const char *s)
{
        for (size_t i = 0; i < sizeof(g_kwds)/sizeof(*g_kwds); ++i) {
                if (!strcmp(s, g_kwds[i])) return 1;
        }
        return 0;
}
